# nix-owo

Brute force Nix source hashes to find one that ends in `0w0`. Point it at a clean source tree containing a README.md file and it will generate an HTML comment you can add to the top of your README.md that will cause the entire source tree to yield the 0w0 hash. `.git` is automatically excluded, just make sure the source tree is clean. A match will be found after about 100,000 iterations (about 30 seconds for small projects).

## Usage

```
usage: nix-owo [TARGET = .] [OPTIONS]

Available options:
  -h [ --help ]          show usage information
  -j [ --jobs ] arg (=1) number of parallel jobs to run
```
