# Configuration Generator for Execution-Manager

## Config items

The execution-managers are parameterized with JSON config files. The config files contain an array `containers` of elements. Each element represents an container/application.

```json
"containers":
[
    {
        "name": "container01",
        "command": "start"
    },
    {
        "name": "container02",
        "command": "start"
    }
]
```

The different execution-managers rely on different sets of key/value pairs for necessary command.

### Execution-Manager (containerd)

| command | name | runtime | digest | image | oci-spec |
| ------- |:----:|:-------:|:------:|:-----:|:--------:|
| create | x | x | x | x | x |
| start | x | | | | |
| run | x | x | x | x | x |
| killAndDelete | x | | | | |

### Execution-Manager (crun)

| command | name | mount_options | mount_target | bundle |
| ------- |:----:|:-------------:|:------------:|:------:|
| mount | x | x | x | |
| umount | x | | | |
| create | x | | | x |
| start | x | | | |
| run | x | x | x | x |
| delete | x | | | |
| kill | x | | | |

### Execution-Manager (native)

| command | name | config |
| ------- |:----:|:------:|
| `<path-to-executable>` | x | x |

## Examples

Create configs for all tests: \

* containerd tests: `python3 ./generate_configs.py -i <image:tag> -n <number-of-containers> -t containerd -b ../benchmarks/bundles -o ../benchmarks/configs -c all`
* crun tests: `python3 ./extract_layers.py -i <image:tag> -o ../benchmarks/image && sudo python3 ./generate_configs.py -l ../benchmarks/image -n <number-of-containers> -b ../benchmarks/bundles -t crun -o ../benchmarks/configs -c all`
* native tests: `python3 ./generate_configs.py -n <number-of-instances> -b ../benchmarks/bundles -t native -o ../benchmarks/configs -c <path-to-executable>`
