# nix-owo

Brute force Nix source hashes to find one that ends in `0w0`. Point it at a clean source tree containing a README.md file and it will generate an HTML comment you can add to the top of your README.md that will cause the entire source tree to yield the 0w0 hash. `.git` is automatically excluded, just make sure the source tree is clean. A match will be found after about 100,000 iterations (about 30 seconds for small projects).

## Usage

`nix-owo [TARGET = .] [TOTAL = 1] [RANK = 1]`

- `TARGET` is the path to the source tree whose hash you want to brute force.
- `TOTAL` is the number of processes you are running. Used to compute the starting and ending number for searching.
- `RANK` is a number uniquely identifying this process from parallel processes. Used to compute the starting and ending number for searching. Should be in the range 1...TOTAL.

## Usage with GNU Parallel

```bash
parallel --halt now,success=1 ./result/bin/nix-owo ~/your-project $(nproc) ::: $(seq 1 $(nproc))
```
