# The NanoShell64 Operating System

#### EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL

NanoShell64 is a 64-bit operating system designed with SMP in mind.

The project is wholly licensed under the GNU General Public License V3, **except the following**:
- [Limine Terminal](source/LimineTerm): https://github.com/limine-bootloader/terminal

#### Be advised this is alpha-level software and you should not expect any stability from it.

NOTE: You need Limine V3.18.3, 3.0 or older doesn't work for some reason

## Building
If you can build [the Limine Bare-bones kernel], surely you can build this too (type `make`). :)

## Goals/plans

#### Architecture design
There's hardly a decided architecture design, as this is right now at the experimental stage.
I'd like to experiment with: (currently only one thing but I may add more)
* Worker thread centered design. This can also be interpreted as a clients-server architecture.
* Each CPU has its own kernel heap. This reduces TLB shootdowns. If there's a need to transfer
  data between CPUs, one may use an IPI with a list of physical pages.
* Until swapping to disk is added, use a form of poor man's compression - if a page is filled
  to the brim with a single byte that fills unspecified criteria, it'll be "compressed" down
  into a single page entry.

#### Primordial tasks
* [x] Hello World
* [ ] SMP Bootstrap
* [ ] Inter-processor communication (through IPIs)
* [ ] Task switching and concurrency
* [ ] Physical memory manager
* [ ] Virtual memory manager
* [ ] Inter-process communication

#### Other features
* [ ] Init ram disk file system
* [ ] Ext2 file system support
* [ ] More... (still not decided)

#### Drivers
* [ ] Limine terminal
* [ ] PS/2 Keyboard
* [ ] Own terminal with framebuffer
* [ ] Serial port
* [ ] PCI
* [ ] PS/2 mouse

#### User
* [ ] A basic shell
Still to be decided.

#### Far in the future
* [ ] NanoShell32 compatibility
* [ ] Networking?
* [ ] USB (could also backport to NanoShell32 itself)

