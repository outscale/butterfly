#!/bin/bash
set -e

function usage {
    echo "This script connects to a butterfly server and provide the current"
    echo "state of butterfly (nics, configuration, security groups, etc..."
    echo ""
    echo "Usage: show_butterfly_state.sh OPTIONS"
    echo "OPTIONS:"
    echo "    -i | --ip      : Butterfly's ip (default: 127.0.0.1)"
    echo "    -p | --port    : Butterfly's port (default: 9999)"
    echo "    -c | --client  : Butterfly's client (mandatory)"
    echo "    -l | --loop    : Continuously ask Butterfly status"
    echo "    -h | --help    : show this help"
}

# Read arguments
ip="127.0.0.1"
port=9999
client=""
loop=0
args=`getopt -o i:p:c:l::h:: --long ip:,port:,client::,loop::,help:: -- "$@"`
eval set -- "$args"
while true ; do
    case "$1" in
        -i|--ip)
            ip=$2 ; shift 2 ;;
        -p|--port)
            port=$2 ; shift 2 ;;
        -c|--client)
            client=$2 ; shift 2 ;;
        -l|--loop)
            loop=1 ; shift 1 ;;
        -h|--help)
            usage; exit 0 ;;
        --)
            break ;;
        *)
            usage; exit 1 ;;
    esac
done

# Check arguments
if [ -z "client" ]; then
    echo -e "Error: Butterfly client is not set\n"
    usage
    exit 1
fi

if ! $client --help &> /dev/null ; then
    echo -e "Error: Butterfly's client path is not correct"
    usage
    exit 1
fi

function client_run {
   local input=$1
   $client -e tcp://$ip:$port -i $input -s
}

nics=()
function nic_list {
    local tmp=/tmp/client.req
    echo -e "
    messages {
  revision: 0
  message_0 {
    request {
      nic_list: true
    }
  }
}" > $tmp
    local r=$(client_run $tmp)
    nics=($(echo $r | cut -d '}' -f 2 | sed -e 's/nic_list/ /g' | tr -d ':"' | tr -s ' '))
}

function nic_details {
    echo -e "# NICS"
    for nic in "${nics[@]}"; do
        local tmp=/tmp/client.req
        echo -e "
    messages {
  revision: 0
  message_0 {
    request {
      nic_details: \"$nic\"
    }
  }
}" > $tmp
        local r=$(client_run $tmp)
        echo $r | cut -d '{' -f 6 | cut -d '}' -f 1
    done
}

sgs=()
function sg_list {
    local tmp=/tmp/client.req
    echo -e "
    messages {
  revision: 0
  message_0 {
    request {
      sg_list: true
    }
  }
}" > $tmp
    local r=$(client_run $tmp)
    sgs=($(echo $r | cut -d '}' -f 2 | sed -e 's/sg_list/ /g' | tr -d ':"' | tr -s ' '))
}

function sg_details {
    echo -e "# SECURITY GROUPS RULES"
    for sg in "${sgs[@]}"; do
        sg_rule_list $sg
    done
    echo -e "# SECURITY GROUPS MEMBERS"
    for sg in "${sgs[@]}"; do
        sg_member_list $sg
    done
}

function sg_rule_list {
    local sg=$1
    local tmp=/tmp/client.req
    echo -e "
    messages {
  revision: 0
  message_0 {
    request {
      sg_rule_list: \"$sg\"
    }
  }
}" > $tmp
    local r=$(client_run $tmp)
    echo "$sg: $r"
}

function sg_member_list {
    local sg=$1
    local tmp=/tmp/client.req
    echo -e "
    messages {
  revision: 0
  message_0 {
    request {
      sg_member_list: \"$sg\"
    }
  }
}" > $tmp
    local r=$(client_run $tmp)
    echo "$sg: $r"
}

function get_infos {
    nic_list
    nic_details
    sg_list
    sg_details
}

get_infos

