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

- `Connection_state` — pure state machine (no side effects) tracking `State` and `reason_`. Used by both client and server `Tcp_connection`. Default-constructs to `State::connecting`; the server uses the parameterized constructor (`State::connected`) since it's born post-accept. Functions: `set_state` (free transitions), `initiate_disconnect` (state → disconnecting), `disconnect` (state → disconnected). The disconnect variants return whether the state changed; callers handle socket close and callbacks.
- `Socket` — wraps Asio TCP socket, calls back into a `Socket::Handler`. Tracks pending async ops (`connect_pending_`, `read_pending_`, `write_pending_`) and signals `closed()` on the Handler once all ops have drained after `close()`. Async ops are no-ops when `closing_` is set.
- `Heartbeat_timer` — periodic timer. Owns its own `asio::steady_timer`. Signals `heartbeat_send_due` (no traffic this period), `heartbeat_receive_timeout` (no peer activity for the timeout window), `heartbeat_timer_error`, and `heartbeat_timer_stopped` (drain signal, posted after `stop()` so the holder can safely destroy). Tracks `send_count_`/`receive_count_`; the caller increments per packet-type rules from `Tcp_connection`'s `send_packet` and per-packet `process_*` handlers.
- `Login_timer` — one-shot timer. Owns its own `asio::steady_timer`. Single timeout configured at construction. Handler callbacks: `login_timer_error`, `login_timer_expired`, `login_timer_stopped` (drain signal, same contract as the heartbeat one).
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

## Session validation (client)

- `Connection::on_login_success` validates `response.session` against `session_`: if `session_` is non-empty and they differ, disconnect with `Disconnect_reason::session_mismatch`. Empty `session_` accepts any value and locks it in for future reconnects.
- One predicate covers three cases: first login with a specific session requested (must match), first login with empty session (accept anything, lock in), reconnect after a successful login (must match the previously-locked-in value).
- Checked before sequence number validation — session identity is more fundamental than sequence positioning, so fail-fast at the session level before doing sequence math.
- No rollover handling. If the server changes the session, the client disconnects with `session_mismatch` rather than attempting a transition. Revisit if rollover ever becomes a real use case.

## Heartbeat tracking

Periodic activity monitoring while logged in. Each `Tcp_connection` owns a `Heartbeat_timer`, starts it at the transition to `logged_in`, stops on disconnect.

**Timeouts are named by what we wait for, not who uses it.** `client_heartbeat_timeout` = "timeout waiting for client heartbeats" — server uses it. `server_heartbeat_timeout` — client uses it.

**Send counting** (`increment_send_count`): only the per-side data packet — `Unsequenced_data_packet` (client) / `Sequenced_data_packet` (server). Heartbeats, login, logout, debug excluded. Counting the heartbeat we just sent would defeat the elision-when-other-traffic rule and produce every-other-period heartbeats. Counted at queue success (`send_packet` on `Write_error::none`), not at write completion (`write_success`) — keeps the count aligned with send intent so a packet still queued suppresses its window's heartbeat instead of triggering a redundant one behind it.

**Receive counting** (`increment_receive_count`): data, heartbeat, end-of-session. Debug excluded under the narrow "real protocol activity" reading. Pre-login packets are rejected as `unexpected_sequence` rather than counted — heartbeat tracking only runs while logged in.

**`heartbeat_send_due`** — Tcp_connection sends the side's heartbeat packet with `(void)socket_.async_write(...)`, discard failure as best effort (buffer-full implies other traffic flowing, closing implies we're shutting down anyway).

**`heartbeat_receive_timeout`** — Tcp_connection disconnects with `Disconnect_reason::heartbeat_timeout`.

## Login timer

One-shot per side, watching for the next login-flow packet.

- **Server**: starts in `Tcp_connection`'s constructor (post-accept, pre-login), waits `login_request_timeout` for `Login_request_packet`. Stops at the top of `process_login_request` after size/state validation succeeds.
- **Client**: starts in `connect_success` after queuing `Login_request_packet`, waits `login_response_timeout` for `Login_accepted_packet` / `Login_rejected_packet`. Stops at the top of `process_login_accepted` / `process_login_rejected` after size/state (and field) validation succeeds.

**Explicit stop after validation**, not at the start of the function — expresses "we received a well-formed login packet." Failure paths (size/state mismatch, malformed field) rely on the disconnect cascade to stop the timer.

**Two separate timers, not a shared `asio::steady_timer`.** Login and heartbeat lifecycles are sequential (login before logged_in, heartbeat after), so sharing would work, but coordinating cancel + re-arm between two helpers introduces fragile ordering between otherwise-independent objects. `asio::steady_timer` is cheap; keep them independent.

**`login_timer_expired`** — Tcp_connection disconnects with `Disconnect_reason::login_timeout`.

## Drain coordination

`Tcp_connection` holds multiple async-owning subsystems (socket, login timer, heartbeat timer). Destroying it synchronously when `Socket::closed()` fires would dereference captured `this` from the other pending operations.

Each subsystem has a paired flag on `Tcp_connection` and posts a drain-complete signal. `maybe_signal_closed` fires the upper-layer `on_closed` only when all are drained:

```cpp
if (!socket_closed_ || !login_timer_stopped_ || !heartbeat_timer_stopped_)
  return;
```

**Defaults reflect the lifecycle:**
- `socket_closed_ = false` — Socket is always active over the connection's life; produces exactly one `closed()` signal.
- `login_timer_stopped_ = true`, `heartbeat_timer_stopped_ = true` — Timers don't always activate. Defaulting "drained" means pre-login disconnects don't wait for signals that won't come. Flipped to false at `start()`, back to true on the `*_stopped` callback.

`disconnect()` calls `subsystem.stop()` on each timer unconditionally — `stop()` is idempotent (early-returns on `!started_`). Order is declaration order: `socket_.close()`, `login_timer_.stop()`, `heartbeat_timer_.stop()`.

## Connection lifecycle and destruction

The closed cascade enables value-owned `Tcp_connection` without `shared_ptr`:

1. `socket_.close()` sets `closing_=true`, cancels pending ops, calls `Socket::maybe_signal_closed`.
2. `Socket::maybe_signal_closed` posts `Socket::Handler::closed()` once all `*_pending_` flags drain (via `asio::post`, never inline).
3. Each owned timer (`login_timer_`, `heartbeat_timer_`) follows the same pattern after `stop()` — drains its pending `async_wait`, posts a `*_stopped` signal.
4. `Tcp_connection::closed()` and each `*_timer_stopped` callback set their respective drain flag, then call `Tcp_connection::maybe_signal_closed`. The upper-layer fire happens only when all flags indicate drained (see [Drain coordination](#drain-coordination)).
5. From `Tcp_connection::maybe_signal_closed`:
   - Client: `Connection::on_closed(reason)` resets the `std::optional<Tcp_connection>` and fires disconnect on `Connection_handler`.
   - Server: `Acceptor::on_closed(connection, port_handler, reason)` removes from `connections_` list and fires disconnect on the appropriate handler — `Port_handler` if logged in, `Acceptor_handler` otherwise.
6. Synchronous destruction inside `Tcp_connection::maybe_signal_closed` is safe because all sources have posted their drain signals before this point.

Disconnect callback timing: fires **from the closed cascade** (post-drain across all sources), not from `Tcp_connection::disconnect`. The state machine's transition to `disconnected` is decoupled from the user notification.

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
- Timer setup (`<timer>_stopped_ = false` flag flip + `<timer>.start()`) goes after state changes and callbacks but before any `socket_.async_*` calls. Convention plus defensive against hypothetical synchronous async completion.
- Timeout constants are named by *what we wait for*, not who uses it: `client_heartbeat_timeout` = "timeout waiting for client heartbeats" → server uses it; `server_heartbeat_timeout` → client uses it. Same shape for `login_request_timeout` (server-side wait) / `login_response_timeout` (client-side wait).
- Packet field validation belongs at the packet layer (`Packet_error::invalid_field_value`), not in the application layer. State-free comparisons against protocol constants (e.g., `response.next_sequence_number == 0`) live in `process_login_accepted` after `read(...)`. State-dependent comparisons (`session_mismatch`, `sequence_number_too_low/high/ahead`) live in `Connection::on_login_success` because they need Connection state. Connection-owned flags consulted by the packet layer (e.g., `has_session_ended_` from `process_sequenced_data`) are queried via public access functions so packet-validity completes at the packet layer before side effects.
- `Login_timer`/`Heartbeat_timer` callers must wait for the `*_stopped` signal before destroying the helper — pending `async_wait` from the owned timer would otherwise dereference dangling memory.
