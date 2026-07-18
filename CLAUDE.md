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
- `Reconnect_timer` — one-shot per arm, re-armable (delay passed per `start()` rather than fixed at construction). Owns its own `asio::steady_timer`. Handler callbacks: `reconnect_timer_error`, `reconnect_timer_expired`. **No `*_stopped` drain signal** — the holder (`Connection`) isn't destroyed mid-life, so per-class drain coordination isn't needed; pending `async_wait` lambdas drain via the io thread join (per `Io_context_runner`). Each arm bumps `epoch_` and stamps its completion lambda; stale completions from prior arms are filtered before reaching `on_expiry` — protects against cross-arm leakage when `stop()` followed by `start()` races the completion queue.
- `types.h` — shared enums: `Disconnect_reason`, `Packet_error`, `Login_reject_reason`, `Write_error`. `Disconnect_reason::none` and similar `none` enumerators serve as sentinels (no value / success), avoiding `std::optional`.
- `error.h` — `Error`, the project's single `std::error_code`-integrated enumeration (the only error type with a category, `soup_category()`), kept separate from the plain enums in `types.h`. See [Error category](#error-category).

## Error category

`Error` (in `error.h`) is the project's only `std::error_code`-backed enumeration — the sole error type carrying a `std::error_category` (`soup_category()`, name `"soup"`). The plain enums in `types.h` are handler/internal enums with no category. `Error` lives in its own header because it carries the `std::is_error_code_enum` specialization and the category.

- **Code enum, not condition enum.** `is_error_code_enum<Error>` is specialized; `is_error_condition_enum` is not. `Error` therefore converts *implicitly* to `std::error_code`. Enumerators start at 1 — value 0 is reserved for "no error".
- **Generic name kept deliberately.** `Error` rather than `Config_error`: the `bc::soup` namespace already supplies context, and it's the sole code enum. Revisit only if a second category is ever added — at which point name *that* one specifically and reconsider `Error`. `Error_code` would mislabel the enum as the `std::error_code` wrapper rather than its payload — and collide with it. `soup_category()` / `"soup"` follow the std convention (`<domain>_category()`, single word); a second category could move the names to a dotted form (`"soup.config"`, …).
- **Membership.** All `Error` values are configuration errors — raised while configuring or starting a client/server, not during live operation. Three kinds: *validation* (`username_too_long`/`invalid_username` and the password/session equivalents — a malformed value), *collision* (`endpoint_in_use`, `username_in_use` — a well-formed key already in use), and *precondition* (`handler_not_set` — `start()` called before setup is complete).
- **`default_error_condition` maps to `std::errc` only on a genuine semantic match.** Validation errors → `invalid_argument`; `endpoint_in_use` → `address_in_use`. `username_in_use` and `handler_not_set` have no honest portable equivalent, so they map to themselves in the soup category. Consequence: the catch-all for "any soup configuration error" is `ec.category() == soup_category()`.
- **Validators.** `validate_username` / `validate_password` / `validate_session` (in `validate.h`) return `std::error_code` carrying `Error` (size → `*_too_long`, character set → `invalid_*`, else `{}`). Size is checked before character set, so an over-long *and* malformed input reports `*_too_long`.

## Client side

Hierarchy: `Client` → `Connection` → `Tcp_connection`

- `Client` — owns a list of `Connection` objects. Designed for **one logical connection with redundant physical connections** — multiple connections exist for redundancy, not for independent sessions. Packets already received by one connection are dropped by others. If multiple independent sessions are needed, users create multiple `Client` instances.
- `Connection` — represents one physical TCP connection. Holds credentials (username, password, session), state flags (e.g. `has_session_ended_`), owns a `Tcp_connection`, and drives automatic reconnect via a `Reconnect_timer` member (inherits `Reconnect_timer::Handler`). Sits between `Client` and `Tcp_connection` in the call chain.
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
- `Write_packet` copy constructor is deleted to prevent accidental copies. The named `clone()` helper performs the explicit copy needed when sending to multiple connections — `Write_packet::clone` delegates to `Buffer::clone`, a faithful single-`memcpy` duplicate of the full backing buffer (header, payload, and any spare capacity). Fidelity lives on `Buffer`, where the `null ⟹ size 0` invariant is, keeping `Write_packet::clone` a trivial correct-by-construction delegation; `Buffer` likewise deletes its copy constructor, with `clone()` as its named explicit-copy sibling. A shrink-to-fit `trim` is left as a separate operation if ever needed, called before `clone`.
- `Client` dispatches to `send_one`, `send_two`, or `send_multiple` based on connection count. `send_one` and `send_two` are fast paths for the common cases (1 or 2 redundant connections).

## Send logout request (client)

- Two layers. `Connection::send_logout_request` checks `connection_` then delegates to `Tcp_connection::send_packet` — the single state-gated entry point for fire-and-forget protocol packets. `Client::send_logout_request` iterates all connections. (Server-side `Port::end_session` follows the same shape for `End_of_session_packet`.)
- No `has_session_ended_` guard at `Connection` — logout is a transport-level action; the `logged_in` state check inside `send_packet` is the meaningful gate.
- `Client::send_logout_request` returns `void` (user is closing anyway; per-connection error handling available via `Connection` layer if needed).
- No `asio::post` — same contract as `send_message` (user's responsibility to be on the io thread).

## Process logout request (server)

- `Tcp_connection::process_logout_request` validates payload size and `logged_in` state, fires `handler_->logout_request()`, then `disconnect(Disconnect_reason::logout_request)`.
- No `has_session_ended()` guard on the logout notification — logout is real intent during the post-session-end tail (replay, pre-shutdown). Differs from `process_unsequenced_data` which suppresses delivery after session end.
- User sees two notifications per logout: `logout_request()` at packet receipt, then the disconnect callback with `Disconnect_reason::logout_request` from the closed cascade.

## Connection takeover (server)

When a login **succeeds** for a username with an existing `connection_` set on the matching `Port`, the prior connection is **superseded**: `Port::on_login_request` calls `connection_->supersede()` — after the password/session checks pass — before reassigning `connection_`. A rejected login leaves the existing connection untouched (an invalid login can't knock off the live one). `supersede()` is a sibling of `close()` — both delegate to `disconnect()`, differing only in the `Disconnect_reason` carried.

`Port::on_closed(Tcp_connection&)` is identity-aware: the bumped connection's later drain calls `port_->on_closed(*this)` with the *old* connection, which doesn't match the (now-new) `connection_` and is a no-op. Without the check, the old drain would clobber the new pointer.

User-visible callbacks (server side, same `Port_handler` for both sessions):
- `login_success(...)` for the new session fires synchronously from `on_login_request`.
- `disconnect(Disconnect_reason::superseded)` for the prior session fires later from the closed cascade.

The bumped connection's *client* sees `disconnect(Disconnect_reason::peer_closed)` — the protocol has no "you were superseded" packet, so the client only knows its peer dropped.

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
   - Client: `Connection::on_closed(reason)` resets the `std::optional<Tcp_connection>` — destroying the `Tcp_connection` — and fires disconnect from `Connection` on `Connection_handler`.
   - Server: `Acceptor::on_closed(connection, port_handler, reason)` removes `connection` from the `std::list<Tcp_connection>` — destroying the `Tcp_connection` — and fires disconnect from `Acceptor` on the appropriate handler — `Port_handler` if logged in, `Acceptor_handler` otherwise.
6. Synchronous destruction inside `Tcp_connection::maybe_signal_closed` is safe because all sources have posted their drain signals before this point.

Disconnect callback timing: fires **from the closed cascade** (post-drain across all sources), not from `Tcp_connection::disconnect`. The state machine's transition to `disconnected` is decoupled from the user notification.

## Public connect/close (client `Connection`)

`Connection::connect()` and `Connection::close()` are public. Users can individually manage a redundant physical connection (e.g. close a misbehaving one without affecting others).
- `connect()` guarded with `!client_->started()`, `connection_` (already connected), and `reconnect_timer_.started()` (retry already pending) — bails as no-op on any.
- `close()` has no started guard — `connection_` null check is the meaningful gate, and a started guard would interfere with `Client::stop` iterating. Also stops the reconnect timer and resets `reconnect_next_delay_` to zero, so a subsequent `connect()` starts the backoff curve fresh.

## Reconnect (client)

Per-`Connection` automatic reconnect with library-owned backoff. After a retryable disconnect, `Connection::on_closed` schedules a retry through `Reconnect_timer`; the timer fires, `Connection` constructs a fresh `Tcp_connection`, and the next login attempt proceeds. The design is library-owned (not user-driven) because the retryability classification is a library property, not a user preference — same answer for every user. Each redundant physical connection retries independently; there's no Client-level coordination of retries.

**Retryable vs terminal classification.** `is_retryable(Disconnect_reason)` in `connection.cpp`'s anonymous namespace is the single source of truth. Retryable: `peer_closed`, `connect_failure`, `transport_error`, `login_timeout`, `heartbeat_timeout`. All other reasons are terminal — `on_closed` short-circuits before scheduling.

**No retry on reconnect-timer-internal error.** `Connection::reconnect_timer_error` fires `transport_error` to the user and stops — no further `schedule_reconnect`. The retry version was implemented and reverted: if asio's timer infrastructure is broken, the next attempt's `Login_timer`/`Heartbeat_timer` fail too, *and* a persistently throwing `expires_after` tight-loops the io thread — even with `asio::post` between attempts to break sync recursion, each `schedule_reconnect → start → throw → error → schedule_reconnect` round happens back-to-back with the backoff value growing in the callback but no actual wait imposed since the timer can't arm. Absence of `reconnect_scheduled` after this `transport_error` is the "give up" signal; user can call `connect()` directly to re-engage.

**Backoff curve.** Constants in `constants.h`: `reconnect_initial_delay = 1s`, `reconnect_max_delay = 30s`, `reconnect_delay_multiplier = 2`. Progression: 0, 1, 2, 4, 8, 16, 30, 30, … The 0s first step is deliberate — a transient blip deserves an immediate retry. `Connection::reconnect_next_delay_` holds the *next* delay to use (same semantics as `next_sequence_number_`): captured at use, then bumped. Reset to 0s on `on_login_success` (healthy session restarts the curve) and on `close()` (user-initiated reset).

**User contract via `reconnect_scheduled(delay)`.** Fires from inside `on_closed` immediately after `disconnect(reason)`, same call stack, no `asio::post` between them — observers see the two back-to-back or just `disconnect` alone, decisively. Absence of `reconnect_scheduled` following `disconnect` is the terminal signal. The callback fires even for `delay = 0` — the timer still imposes one io tick of delay, and reporting it gives the user a uniform "retry pending" signal across the curve.

**0s goes through the timer, not bypassed.** It's tempting to special-case `delay == 0` as a direct emplace or `asio::post`, but the timer is what gives `close()` a handle to cancel an armed retry and `connect()` a gate against re-entry while a retry is pending.

## Failure handling

Three handler callbacks report transport-layer failures: `connect_failure`, `transport_error`, `protocol_violation`. Each is wired through a `Tcp_connection::handle_*` helper with the same two-line shape — fire the handler callback, then `disconnect(Disconnect_reason::<X>)`. The `disconnect()` call encapsulates state change, socket close, and idempotent timer stops, so the handlers don't open-code any of that.

**Two-notification contract.** User receives the specific failure callback at the failure point, then the disconnect callback from the closed cascade with the matching `Disconnect_reason`.

**Server pre-login routing.** A pre-login-capable callback checks `handler_` (the `Port_handler`): if set (logged in), the per-port handler fires; otherwise the per-acceptor handler (`acceptor_handler_`) fires — or drops, when there's no acceptor-level equivalent. So `debug` / `transport_error` / `protocol_violation` → `Acceptor_handler`; `write_buffer_empty` (port-only) → drop. Client-side always has `handler_` (the `Connection_handler`) set, so no branching.

**Operation strings.** Format is `"<category> <function_name>"`:
- `<function_name>` is the literal C++ identifier of the failed call (`"async_read"`, `"async_write"`, `"expires_at"`, `"async_wait"`).
- `<category>` (`"socket"`, `"timer"`) is included only when the callback spans categories. `transport_error` covers socket and timer failures and carries the prefix. `listen_setup_failure` and `connect_failure` are single-category and omit it.

## Coding conventions

- Async calls (socket_.async_*) go after all state changes and callbacks.
- `socket_.close()` goes after callbacks (disconnect, connect failure).
- Handler call after state change — but only on *success / state-advancing* paths, where the handler may immediately act on the newly-valid connection. *Failure / teardown* paths deliberately invert it: the notification fires the moment the outcome is known, against a still-coherent connection (socket open, timers live, state unchanged), and `disconnect(reason)` runs last as the final lifetime step. A user acting on the connection from inside the notification therefore sees the live connection that just failed, not one that's already been torn down.
- Upper-layer calls before handler callbacks.
- Handler notifications fire at the lowest layer that reaches the handler, moving up the hierarchy only when an upper layer owns the context the call needs (server `login_failure` — the reject reason is decided at `Acceptor`/`Port`). Reaching a handler is not a reason to go up (hold the handler pointer); reading a value for a filter is not either (use an access function). Destroy-time: `disconnect` fires from the owner (`Connection` / `Acceptor`), not `Tcp_connection`, because closing destroys the `Tcp_connection`.
- `[[nodiscard]]` reserved for error types or `expected` (error-bearing returns). State machine bool returns are not marked nodiscard — they're informational/optimizational, ignoring is not a bug.
- `Connection_state` return stored as `state_changed`; not inlined into if-condition since the function has side effects.
- No default parameter on `Connection_state::disconnect` — callers always pass an explicit reason. The no-arg `Tcp_connection::disconnect()` exists for `read_aborted` (socket closed from our side, `reason_` always set).
- `initiate_disconnect` is server-only now, narrowed to the graceful login-rejected path and renamed `prepare_graceful_disconnect` at `Tcp_connection`. Non-graceful paths transition straight to disconnected via `disconnect()`.
- Re-arm of `socket_.async_read()` in `read_success` is guarded with `!state_.is_closing()` — early returns when closing so no reads are queued after the close decision.
- Timer setup (`<timer>_stopped_ = false` flag flip + `<timer>.start()`) goes after state changes and callbacks but before any `socket_.async_*` calls. Convention plus defensive against hypothetical synchronous async completion.
- Timeout constants are named by *what we wait for*, not who uses it: `client_heartbeat_timeout` = "timeout waiting for client heartbeats" → server uses it; `server_heartbeat_timeout` → client uses it. Same shape for `login_request_timeout` (server-side wait) / `login_response_timeout` (client-side wait).
- Packet field validation belongs at the packet layer (`Packet_error::invalid_field_value`), not in the application layer. State-free comparisons against protocol constants (e.g., `response.next_sequence_number == 0`) live in `process_login_accepted` after `read(...)`. State-dependent comparisons (`session_mismatch`, `sequence_number_too_low/high/ahead`) live in `Connection::on_login_success` because they need Connection state. Connection-owned flags consulted by the packet layer (e.g., `has_session_ended_` from `process_sequenced_data`) are queried via public access functions so packet-validity completes at the packet layer before side effects.
- `Login_timer`/`Heartbeat_timer` callers must wait for the `*_stopped` signal before destroying the helper — pending `async_wait` from the owned timer would otherwise dereference dangling memory. `Reconnect_timer` is exempt because its holder (`Connection`) isn't destroyed mid-life; pending waits drain via io thread join.
- Timer error callbacks are suppressed once `stop()` has been called. Applies uniformly to `Login_timer`, `Heartbeat_timer`, and `Reconnect_timer`: `stop()`'s cancel catch swallows the error silently, and `on_expiry`'s `!started_` gate at the top drops aborted, success-that-raced-cancel, and error-that-raced-cancel uniformly. Reasoning: once the holder has signalled "done with this timer," nothing the timer produces is actionable — routing the error through the handler would cause a redundant `disconnect(transport_error)` cascade, or worse, an unwarranted one that tears down the connection during a happy-path stop (e.g., login → heartbeat). A side effect of dropping the explicit `!= operation_aborted` check: aborted *without* a prior `stop()` (which shouldn't happen in normal flow) is now surfaced as an error, making unexpected cancellation visible.
