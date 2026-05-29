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

- `Connection_state` — pure state machine (no side effects) tracking `State` and `reason_`. Used by both client and server `Tcp_connection`. Functions: `initiate_disconnect` (state → disconnecting), `disconnect` (state → disconnected). Both return whether the state changed; callers handle socket close and callbacks.
- `Socket` — wraps Asio TCP socket, calls back into a `Socket::Handler`. Tracks pending async ops (`connect_pending_`, `read_pending_`, `write_pending_`) and signals `closed()` on the Handler once all ops have drained after `close()`. Async ops are no-ops when `closing_` is set.
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

## Send logout request (client)

- Two layers. `Connection::send_logout_request` checks `connection_` then delegates to `Tcp_connection::send_packet` — the single state-gated entry point for fire-and-forget protocol packets. `Client::send_logout_request` iterates all connections. (Server-side `Port::end_session` follows the same shape for `End_of_session_packet`.)
- No `has_session_ended_` guard at `Connection` — logout is a transport-level action; the `logged_in` state check inside `send_packet` is the meaningful gate.
- `Client::send_logout_request` returns `void` (user is closing anyway; per-connection error handling available via `Connection` layer if needed).
- No `asio::post` — same contract as `send_message` (user's responsibility to be on the io thread).

## Process logout request (server)

- `Tcp_connection::process_logout_request` validates payload size and `logged_in` state, fires `handler_->logout_request()`, then `disconnect(Disconnect_reason::logout_request)`.
- No `has_session_ended_` guard on the logout notification — logout is real intent during the post-session-end tail (replay, pre-shutdown). Differs from `on_unsequenced_data` which suppresses delivery after session end.
- `handler_->logout_request()` called directly from `Tcp_connection`, not via `port_->on_logout_request()` — no Port-state gate to apply. Parked under "Handler call conventions" for the broader review.
- User sees two notifications per logout: `logout_request()` at packet receipt, then the disconnect callback with `Disconnect_reason::logout_request` from the closed cascade.

## Sequence number tracking (client)

- `Client::next_sequence_number_` (default 1, settable via `set_next_sequence_number`) is the canonical counter. Each `Connection` holds its own `next_sequence_number_` for per-physical accounting.
- On `on_connect_success`, Connection pulls Client's current value and sends it in `Login_request_packet`. On `on_login_success`, validates `response.next_sequence_number`: non-0 request requires exact match (`sequence_number_too_low`/`too_high`); 0 request bootstraps Client's counter on the first response and requires `<= Client::next` on subsequent (else `sequence_number_ahead_of_session`); `response == 0` is `protocol_violation`.
- `Connection::on_sequenced_data` labels each incoming packet with its current counter then bumps, delivers `(seq, data, size)` to `Client`. `Client::on_sequenced_data` drops `seq < next`, otherwise advances `next = seq + 1` and forwards.
- **Invariant:** `Connection::next_ <= Client::next` always — enforced by login validation, preserved by lockstep advancement. Makes `seq > Client::next` provably unreachable; the assert in `Client::on_sequenced_data` documents this.

## Sequence number tracking (server)

- `Port::next_sequence_number_` (default 1, settable via `set_next_sequence_number`) is the per-port counter.
- `Port::send_message` bumps the counter on `send_packet` success (`Write_error::none` only — `buffer_full` does not bump). The counter is the message's implicit sequence number; it isn't carried in the packet payload.
- `Port::on_login_request`: if `request.next_sequence_number != 0 && < current`, rewinds the counter to the requested value (replay scenario). Otherwise leaves it alone. Response always carries the (possibly rewound) counter value.
- **No library-side message store.** The user holds payloads keyed by sequence number and feeds them via `send_message`. For replay, the user observes `login_accepted.next_sequence_number` and feeds messages until caught up to their canonical, then resumes live.
- Counter commits eagerly at send (not at write-completion). Messages assigned a sequence number via `send_message` keep that number even if the connection dies before flush; recovery is via replay on the next connection.

## Connection lifecycle and destruction

The closed cascade enables value-owned `Tcp_connection` without `shared_ptr`:

1. `socket_.close()` sets `closing_=true`, cancels pending ops, calls `maybe_signal_closed`.
2. `maybe_signal_closed` posts `Handler::closed()` once all `*_pending_` flags drain (via `asio::post`, never inline).
3. `Tcp_connection::closed()` fires the user's disconnect callback (post-drain) and triggers destruction:
   - Client: `Connection::on_closed(reason)` resets the `std::optional<Tcp_connection>` and fires disconnect on `Connection_handler`.
   - Server: `Acceptor::on_closed(connection, port_handler, reason)` removes from `connections_` list and fires disconnect on the appropriate handler — `Port_handler` if logged in, `Acceptor_handler` otherwise.
4. Synchronous destruction inside `closed()` is safe because `closed` always arrives from a posted dispatch, so the socket handlers have already unwound.

Disconnect callback timing: fires **from the closed cascade** (post-drain), not from `Tcp_connection::disconnect`. The state machine's transition to `disconnected` is decoupled from the user notification.

## Public connect/close (client `Connection`)

`Connection::connect()` and `Connection::close()` are public. Users can individually manage a redundant physical connection (e.g. close a misbehaving one without affecting others).
- `connect()` guarded with `!client_->started()` and `connection_` (already connected) — bails as no-op.
- `close()` has no started guard — `connection_` null check is the meaningful gate, and a started guard would interfere with `Client::stop` iterating.

## Handler call conventions (unresolved)

Two related questions are deferred to a single review, once all message types are implemented:

1. **Layer.** Handler calls are made from both `Tcp_connection` and the upper layer (`Connection` on the client side, `Port` / `Acceptor` on the server side). No firm convention has been chosen — examples of both patterns exist on both sides, and some `Tcp_connection`-level handler calls are placed there deliberately. To be decided as part of the review.

2. **Order.** The rule is handler call after state change. Currently violated in:
   - **Server login (accept and reject):** `Port::on_login_request` fires `handler_->login_success` / `handler_->login_failure` before `Tcp_connection::process_login_request` performs the post-decision state transition (`set_state(logged_in)` or `prepare_graceful_disconnect`).
   - **Client login rejected:** `Tcp_connection::process_login_rejected` fires `handler_->login_failure` before calling `disconnect(Disconnect_reason::access_denied)`.
   - **Server logout request:** `Tcp_connection::process_logout_request` fires `handler_->logout_request` before calling `disconnect(Disconnect_reason::logout_request)`.

Resolving (2) on the server side may interact with (1) — if handler calls remain in the upper layer, that layer would need to drive the state transition (e.g., calling back into `Tcp_connection` to set state before firing the handler).

## Coding conventions

- Async calls (socket_.async_*) go after all state changes and callbacks.
- `socket_.close()` goes after callbacks (disconnect, connect failure).
- State changes before callbacks; upper-layer calls before handler callbacks.
- `[[nodiscard]]` reserved for error types or `expected` (error-bearing returns). State machine bool returns are not marked nodiscard — they're informational/optimizational, ignoring is not a bug.
- `Connection_state` return stored as `state_changed`; not inlined into if-condition since the function has side effects.
- No default parameter on `Connection_state::disconnect` — callers always pass an explicit reason. The no-arg `Tcp_connection::disconnect()` exists for `read_aborted` (socket closed from our side, `reason_` always set).
- `initiate_disconnect` is server-only now, narrowed to the graceful login-rejected path and renamed `prepare_graceful_disconnect` at `Tcp_connection`. Non-graceful paths transition straight to disconnected via `disconnect()`.
- Re-arm of `socket_.async_read()` in `read_success` is guarded with `!state_.is_closing()` — early returns when closing so no reads are queued after the close decision.
