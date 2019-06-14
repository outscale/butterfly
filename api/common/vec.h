/* Copyright 2019 Outscale SAS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifdef __cplusplus
#define type(v) auto
#else
#define type(v) typeof(v)
#endif

#define V_FOREACH_(vec, val, cnd, l)                                    \
        type(vec) V_CAT_(val, l) = vec;					\
        for (type(*vec + 0) val = *vec; (cnd);				\
             val = *(++V_CAT_(val, l)))

#define V_FOREACH(vec, val)                                             \
        V_FOREACH_(vec, val, (val), __LINE__)

#define V_FOREACH_UNTIL(vec, val, condition)                            \
        V_FOREACH_(vec, val, condition, __LINE__)

#define V_CAT_(a,b) a ## b

#define V_FOREACH_IF_(vec, val, if_cnd, u_cnd, l)                       \
        type(vec) V_CAT_(val, l) = *&vec;                             \
        for (type((*vec) + 0) val = *vec;                             \
             ({  do {val = *V_CAT_(val, l)++;}                          \
                     while (!(if_cnd));					\
                     val;                                               \
             }) && (u_cnd); )

#define V_FOREACH_IF(vec, val, condition)               \
        V_FOREACH_IF_(vec, val, condition, !(val), __LINE__)

#define V_FOREACH_IF_UNTIL(vec, val, if_cnd, u_cnd)     \
        V_FOREACH_IF_(vec, val, if_cnd, u_cnd, __LINE__)
