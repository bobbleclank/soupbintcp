# soupbintcp

A C++ implementation of the SoupBinTCP protocol, built on Asio.

## Structure

```
include/bc/soup/          - shared types, socket, connection state
include/bc/soup/client/   - client-side public API
include/bc/soup/server/   - server-side public API
src/client/               - client implementation
src/server/               - server implementation
example/                  - example client and server programs
```

## Key shared components

- `Connection_state` — pure state machine (no side effects) tracking `State` and `pending_reason_`. Used by both client and server `Tcp_connection`. Returns values; callers handle socket close and callbacks.
- `Socket` — wraps Asio TCP socket, calls back into a `Socket::Handler`.
- `types.h` — shared enums: `Disconnect_reason`, `Packet_error`, `Login_reject_reason`, `Write_error`. `Disconnect_reason::none` and similar `none` enumerators serve as sentinels (no value / success), avoiding `std::optional`.

## Client side

Hierarchy: `Client` → `Connection` → `Tcp_connection`

- `Client` — owns a list of `Connection` objects. Designed for **one logical connection with redundant physical connections** — multiple connections exist for redundancy, not for independent sessions. Packets already received by one connection are dropped by others. If multiple independent sessions are needed, users create multiple `Client` instances.
- `Connection` — represents one physical TCP connection. Holds credentials (username, password, session), state flags (e.g. `has_session_ended_`), and owns a `Tcp_connection`. Sits between `Client` and `Tcp_connection` in the call chain.
- `Tcp_connection` — handles raw packet I/O, processes login/data packets, delegates up through `Connection` to `Client`.
- `Client_handler` — all callbacks pure virtual; user must implement all.

## Server side

Hierarchy: `Server` → `Acceptor` → `Port` → `Tcp_connection`

- `Server` — owns a list of `Acceptor` objects. Holds the session string (one session per server, shared across all acceptors). Thread-safe calls use `asio::post`.
- `Acceptor` — listens on one endpoint, owns a list of `Port` and `Tcp_connection` objects. Handles accept callbacks.
- `Port` — represents one logical user/session slot (identified by username). Owns the connection pointer and state flags (e.g. `has_session_ended_`). Multiple physical connections can log into the same port (previous connection is logged out on new login).
- `Tcp_connection` — handles raw packet I/O for one accepted socket, delegates up through `Port` to `Acceptor`.

## Connection model asymmetry

| | Client | Server |
|---|---|---|
| Session scope | Per `Connection` | Per `Server` |
| Logical connections | One (redundant physicals) | One `Port` per username |
| Multiple sessions | Create multiple `Client` instances | Not applicable |

## Send message (client)

- `Client::send_message` sends to **all** connections. Returns `Write_error::none` if any connection succeeds.
- `Write_error` priority when all connections fail: `none` > `buffer_full` > `not_logged_in` > `disconnected`. `buffer_full` is transient (retry); others indicate the connection cannot currently be used.
- `Write_packet` copy constructor is deleted to prevent accidental copies. A named `clone()` helper is planned for the explicit copy needed when sending to multiple connections.
- `Client` dispatches to `send_one`, `send_two`, or `send_multiple` based on connection count. `send_one` and `send_two` are fast paths for the common cases (1 or 2 redundant connections).

## Stop at Connection layer (unresolved)

`Connection` currently has no `stop()`. Since connections are individually accessible and redundant, a per-connection hard close would be useful (e.g. a misbehaving connection). Deferred until after logout is implemented.

## Handler call layer conventions (server side — unresolved)

Both `server::Tcp_connection` and `server::Port` hold handler pointers. The intended pattern (handler calls belong in the upper layer, not `Tcp_connection`) is not yet consistently applied. A style review is deferred until all message types are implemented.

## Coding conventions

- Async calls (socket_.async_*) go after all state changes and callbacks.
- `socket_.close()` goes after callbacks (terminate, connect failure).
- State changes before callbacks; upper-layer calls before handler callbacks.
- `[[nodiscard]]` used on functions where ignoring the return is a bug: currently `initiate_disconnect` (returns whether state changed) and `terminate` (returns reason or none).
- `Connection_state::initiate_disconnect` return stored as `state_changed`; not inlined into if-condition since the function has side effects.
- No default parameter on `Connection_state::terminate` — callers always pass an explicit reason; the no-arg `Tcp_connection::terminate()` exists to handle `read_aborted` (socket closed from our side, pending reason always set).
