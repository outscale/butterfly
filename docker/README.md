This Dockerfile help to build butterfly packages in a standard way.

First, we build a docker image:
```
cd ./butterfly
docker build -t butterfly-build ./docker
```

Next, you can build butterfly packages using this image (here you are still in in butterfly's root)
```
docker run --rm -v $(pwd):/butterfly butterfly-build /butterfly/docker/package.sh
```
This will take a while. After a successful build, packages will be available in `packages` folder.
