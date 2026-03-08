# nix-owo

Brute force Nix source hashes to find one that ends in `0w0`. Point it at a clean source tree containing a README.md file and it will generate an HTML comment you can add to the top of your README.md that will cause the entire source tree to yield the 0w0 hash. `.git` is automatically excluded, just make sure the source tree is clean. A match will be found after about 100,000 iterations (about 30 seconds for small projects).

## Usage

`nix-owo [TARGET = .] [START = 1] [END = UINT64_MAX]`

- `TARGET` is the path to the source tree whose hash you want to brute force.
- `START` is the starting number. This should not be 0 as 0 is used for internal testing purposes.
- `END` is the ending number.

## Usage with GNU Parallel

```bash
function gen_args() {
    cores=$(nproc)
    max=$1
    rank=$2
    interval=$((max/cores))
    echo "$((1 + interval * (rank - 1))) $((interval * rank))"
}

function run_owo() {
    cores=$(nproc)
    path=$1
    max=$2
    owo=$3
    rank=$4

    $owo "$path" $(gen_args $max $rank)
}

export -f gen_args
export -f run_owo

parallel --halt now,success=1 run_owo ~/your-project 1000000 ./result/bin/nix-owo ::: $(seq 1 $(nproc))
```

## Known Limitations

It currently does not work well with projects containing multiple README.md files. The computed prefix would need to be inserted at the top of every README.md file in the source tree rather than just the top-level one.
