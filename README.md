# contract-command

The interactor contract in C: a command in, reply bytes out, and nothing about how the command arrived.

## What it is for

An interactor answers one command with no socket, poll loop or knowledge of its transport. A transport moves bytes to and from a wire without knowing what they mean, and a service composes a state, its interactors and its transports. This repository holds only the contract between them, the CBOR reply writer, and an Elixir decoder that turns a reply into a term a caller matches on. The library links nothing, so a transport dependency that creeps into it fails the build. Framing, dispatch and authority belong to the transport, the service and the relations, not here.

## Build and test

```sh
cmake -S . -B build && cmake --build build
ctest --test-dir build
```

The proofs build only when this repository is the top of the tree, not when a service vendors it.

## Licence

Apache-2.0, as the SPDX headers state.
