# Open Process Platform

OPP is an experimental C++ library that tries to use the ideas from
Erlang/Elixir/OTP and adapt them to C++. This creates an optimistic framework
which is highly fault tolerant.

This means:

* Process oriented: The program may have thousands of mini processes that each
  do a small part.
* Message oriented: All processes communicate ONLY by message passing. No shared
  memory, and no thread coordination.
* Let it crash approach: When a process try to do something that may result in an
  error an exception is thrown and the process may stop. A supervisor then
  restarts the process, which ideally know how to start from where it stopped.

Goals:

* Easier multithreading a asynchronous programming.
* No [red/blue](http://journal.stuffwithstuff.com/2015/02/01/what-color-is-your-function/)
  programming. You dont need to know if a function can block (blue) or has to
  be called in a asynchronous mode (red). All functions can block, and your
  code decides how to manage this asynchronicity. And waiting actually means
  that other processes get to go working, so actually the program don't
  stop because you are just waiting network data.
* If a process calls a blocking function, the rest of the process have to
  continue working. To do this there is a standby thread that ensures there
  is process switching going on if there are waiting processes, so that never
  all working threads are waiting for I/O.

But with OPP it will be much easier to program asynchronous code. Actually you
program many small synchronous tasks that interoperate, a system of processes,
which has ensured high reliability.

Non goals and pitfals:

* It is not maximal efficiency at all. Using message passing and context
  switches will never be more efficient that carefully crafted code that
  properly do the same with calls and careful error and exception checking.
  Also it uses exceptions which are more inefficient than plain error return
  codes.
* Segfaults still kill your code. This is important as in Erlang/Elixir there
  are no segfault possible, but with C++ it is high possible with all the
  hidden undefined behaviour (Who knows all undefined behaviour??).

## Use cases

Any code that needs to communicate with several peers at the same time. For
example files, databases, or network connections.

The old way to do this communication is ask for data and wait for answer. Or
wait for request and give the response. Or use select-like functions to be able
to listen to several at the same time. But this is a hell to do properly. What
to do when you are waiting for data from the database then you receive another
request that needs more data from the database. Are you waiting for the database
answer, so the new request has to wait? Use threads and coordination primitives
(mutex?)?

The new wave is about async/await and an event loop. But this has the red/blue
problem.

The OPP way is have one process for each task and make them wait always. While
they wait other processes can do their job. And using message passing there is
no need for more coordination. And as it uses fibers it is more efficient than
OS threads. And as it is actually multithreaded, and with some tricks, if you
use normal blocking code, everything keeps working smoothly.

## Let it crash

Ideas from the [Zen of Erlang](https://ferd.ca/the-zen-of-erlang.html)
presentation.

There are two kind of errors: Heisenbugs and Bohrbugs.

Bohrbugs are plain old bugs. For the same input they always fail in the same
way. These bugs are easy to correct. Just do a test and fix it. Statistically
speaking they probability of they happening is 1.0 for a given function and
data.

But Heisenbugs are bugs that when you look at them they dissapear.
Statistically speaking the probability is less than 1.0. They depend not only
on the inputs but also on the state of the application which is uncertain. And
actually if your program depends on some external input, quite probably this
is the most common of bugs you find. If the probability is very small, lets
say 1e-6 (1 in a million), its very difficult to debug and fix. One would
say almost impossible. But it will happen because computers are fast. Also
there are some conditions that are impossible to avoid, like a network failure.

Of course you should be prepared for most of the possible contingencies your
code may find. But.. what if the way to deal with it is just failing? This means
that you code the happy path all the time. No worries for errors, and when any
error appears, you just fail. And your supervisor starts another process to do
the same job, hopefully this time with a cleaner state or fixed network
connectivity. And if it fails many times, then really signal a failure.. maybe
to another supervisor that knows how to deal with the problem. At process
restart it can just get the same socket descriptor and keep using it, for
example.

Instead of fighting problems, embrace them and use them to control you flow.

If you loose one connection in a million, and this causes the full system to
fail this is a big problem. If you only make this connection to fail you have
still a problem, but it is a smaller one. And if you know how to deal with when
this connection fails to finally provide an answer even if it is trying again,
suddenly you are a hero.

Of course errors are logged so you can inspect and correct the conditions that
lead to it, if required.

To make this all work, its better to have functional data types, where you can
not modify a map, for example, but you create a new map which is the original
with some data changed. This way it is easy to avoid state dependent bugs. And
then at restart of the function, with the same input, if it fails again..
suddenly your Eisenbug is now a Bohrbug. Same input with 1.0 probability of
fail. And that is easier to fix and test.

## Why C++

Exceptions. And many other small things. Other languages as Rust are super cool
with the ownership guaranties, or even Go is half way to OTP, but without
exceptions there is no way to make the *let it crash* mantra work. Or you do all
the error checking, and then why have supervisors, or you have leaks.

With C++ exceptions and RTTI, you are guaranteed that whoever catches the
exception is in a safe state regarding to resource allocation and release.

Also there is the great Boost/Fibers, and all the meta programming that helps in
many places.

## Technical details

Internally it uses a lot of Boost Fibers. So much that actually you can get
almost the same results, maybe more efficiently just using boost fibers. But OPP
adds the supervisors, processes, observer and more. Its a nice layer to make it
even easier to use.

Boost fibers can be used in several threads at the same time, and have
primitives to create queues.

This queues are used as message passing. Internally uses `std::any` as the
message type to send (`process->send(my_struct_msg{...})`). And to receive it
uses a custom match function.

When a *sender* sends a *message* to *receiver* process, it puts the *message*
into the *receiver*'s queue. If there is a match on that message type waiting,
it is processed by the *receiver*. In any case the the  *sender* continues
processing. If there is no match it is enqueued to wait for when the *receiver*
is interested in this *message* type.

There are two common patterns for message sending:

* Send and forget. Just send the message. No wait for anything. This has the
  risk of maybe the *receiver* did not process it. Which in some cases is OK.
  Why did the *receiver* not processed it? Maybe died processing the *message*.
  As OPP is optimistic, it just means that the *sender* is fully asynchronous
  with *receiver* and require no answer.
* Send and wait response. Send a message with a *message* *reference* and
  wait for the *reference* answer. Add to each message a `ref` attribute which
  is a unique id for each message, and wait for that `ref` to come back with
  the answer. And with timeouts just in case it takes long time.

Over these primitives there are many helper classes, as file management,
networking, HTTP servers and clients...

As a control mechanism there are Supervisors, that manage groups of processes
and in case of exception restart one, all, or fail depending on a custom
strategy. Supervisor depend on linked processes and monitoring, which basically
tells a process when another process dies, and the reason. Then this process can
decide to do something about it, like starting it again, or die themselves.

# Example code

The typical echo program (as of 2018-11-25 from examples):

```cpp
#include <string>
#include <vector>
#include <fmt/format.h>
#include "io/tcp/server.hpp"
#include "io/tcp/peer.hpp"
#include "string.hpp"
#include "logger.hpp"
#include "vm.hpp"
#include "task.hpp"

int main(int argc, char **argv){
  opp::start();

  if (argc!=3){
    OPP_ERROR("Requires 3 arguments");
    opp::stop();
    return 1;
  }

  auto server = opp::start<opp::io::tcp::server>(argv[1], argv[2]);

  while(opp::vm->running()){
    auto peer = server->wait_peer();

    opp::task::start([peer]{
      opp::io::buffer_t data(1024);
      while(true){
        peer->read(data);
        peer->write(data);
      }
    });
  }
  server->stop();

  OPP_INFO("EOF");
  opp::stop();
}
```

Basically this example creates a TCP server at the address given by the
arguments to the program, and waits for peers. When a new peer appears, it
creates a new process task, which just reads and writes the same.

There is no need to control for peers closing connection, as if it tries to read
from a closed socket, the read function will fail. The same for write. So when
it closed, it fails, and memory is reclaimed.


# Status

## DONE

* Process
* Message passing
* Async IO / epoll
* TCP Networking
* Basic HTTP Server
* Send/Send-Receive with references
* Message matching
* Improved exception information gathering: Keeps the stacktrace of the
  exception for later inspection.

## DOING

* Observer

## TODO

* Supervisors
* GenServers
* Never block. When processes call blocking code, ensure that there is always
  a process worker ready.

## NOT WORKING

* Multithreading. It should be using all the available threads, but I dont know
  why it always uses the same thread, even when there are other processes waiting
  execution. This requires a custom scheduler (on works).
