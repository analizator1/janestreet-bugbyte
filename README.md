This is a solution to Jane Street [Bug Byte](https://www.janestreet.com/bug-byte/) puzzle.

To build it in debug config (with assertions):
```
cmake .
make
```

This is how it performs:
```
$ time ./bugbyte < bugbyte.in 
Hello world from bugbyte!
Reading data from stdin...
Read all data.
num_vertices: 18
num_edges: 24
num_available_weights: 20
secret_start_vertex: 4
secret_final_vertex: 17
===== found solution =====
...
secret message reversed: ***

real	0m0.823s
user	0m0.821s
sys	0m0.002s
```

For a fast build, use release config:
```
cmake -DCMAKE_BUILD_TYPE:STRING=Release .
make
```

Run it:
```
$ time ./bugbyte < bugbyte.in 
...
secret message reversed: ***

real	0m0.076s
user	0m0.072s
sys	0m0.004s
```
