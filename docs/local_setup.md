# Running a local setup

It is possible to launch a full node locally on a test chain and connect to it via light client to debug the communication on both sides.

## Prerequisites

If you don't have Rust tools please install it (see the manual [here](https://www.rust-lang.org/tools/install))

Also you may need to install protoc if you don't have one (installation instructions [here](http://google.github.io/proto-lens/installing-protoc.html))

## Clone and build polkadot full node

First you'll need a full node:

```
git clone https://github.com/paritytech/polkadot.git
cd polkadot/scripts
./init.sh
cargo build
```

After this polkadot executable should be present in `target/debug` directory.

More detailed build instructions cah be found on polkadots [github page](https://github.com/paritytech/polkadot).

## Create or using existing chain specification

You have two options:

1. Use dev chainspec from Kagome project (can be found [here](https://github.com/soramitsu/kagome/blob/master/examples/first_kagome_chain/localchain.json))

2. Generate your own one with polkadot node: `target/debug/polkadot build-spec --dev > dev.json`. This will create Alice-Bob test blockchain. More details on it can be found [here](https://core.tetcoin.org/docs/en/tutorials/start-a-private-network/alicebob)


## Start a full node

```
/target/debug/polkadot --chain <PATH TO CHAIN SPEC FILE> --listen-addr /ip4/0.0.0.0/tcp/11111
```

This will start a full node listening a TCP port 11111 locally. Please find a string in the output similar to

> Local node identity is: 12D3KooWAeL3xWc7qGEXT96MLyYRentGnXa1sbh9B2xgnFYyY2rt

That is your node identity. To make light node being able to find full node you need to add it to the list of boot nodes in the chain spec file:

> "bootNodes": [
>   "/ip4/127.0.0.1/tcp/11111/p2p/12D3KooWAeL3xWc7qGEXT96MLyYRentGnXa1sbh9B2xgnFYyY2rt"
>  ],

## Start the light client

Just run executable or start debugging passing the same toolchain file:

```
polkadot_light_client <PATH TO CHAIN SPEC FILE>
```

## Debugging full node

You can use visual studio code with [rust tools](https://code.visualstudio.com/docs/languages/rust) installed to debug full node in case when you want to know why the full node rejects connection or smth else. Here is a `.vscode/launch.json`'s content with a debug config that attaches to the `polkadot` process:

```
{
    "configurations": [
    {
        "type": "lldb",
        "request": "attach",
        "name": "Attach",
        "program": "${workspaceFolder}/src/debug/polkadot"
    }
    ]
}
```

However you can easily configure launch too. The main issue is that all the debugging actions are very slow: attaching, pausing on breakpoint and etc. The process itself however runs at more or less normal speed. In `Cargo.toml` I've replaced the original `[profile.dev.package]` section with the following one:
```
[profile.dev.package."*"]
opt-level = 3

[profile.dev.package]
libp2p-swarm = {opt-level = 0}
libp2p-swarm-derive = {opt-level = 0}
libp2p-tcp = {opt-level = 0}
libp2p-wasm-ext = {opt-level = 0}
libp2p-yamux = {opt-level = 0}
multistream-select = {opt-level = 0}
```

Also please note that many things are happening not in polkadot project code but in the dependencies. So it is reasonable to add that third parties to the workspace directly (their sources can be found in a cargo directory `~/.cargo/`).

Apart from debugging `polkadot` has good diagnostics in `trace` log level messages (`debug` seems to be completely useless). You can enable it by adding `-ltrace` in commandline. I advise also redirecting stderr to a file in this case (`2>log.txt`).