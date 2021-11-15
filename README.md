# Dynamic Lock

## Paper

* [Contextual Concurrency Control (HotOS 2021)](https://sigops.org/s/conferences/hotos/2021/papers/hotos21-s08-park.pdf)

## Overview

Kernel synchronization primitives have a huge impact on some applications’
performance and scalability. However, controlling kernel synchronization
primitives is out of reach for application developers.

This work implements a new paradigm that allows fine tune the kernel concurrency
primitives from userspace. With one-time kernel modification along with the
framework, now user can modify kernel locks on the fly without recompiling nor
reboot the whole system.

## Contents

* linux/: linux v5.4 source code with dyanmic-lock patch applied
* dynamic-lock.patch: patch applied to linux v5.4
* concord/: framework to build lock policy and livepatch moule.


## How to run
1. Prepare underlying kernel and policy program using Concord framework.
   First, make sure `./linux` directory is clean v5.4 source code.
   Then run the following script to expose APIs.

```
$ cd concord
$ python3 concord.py -v --linux_src ../linux \
	--lock lockdir/shfllock.patch \
	--policy policy/numa-grouping \
	--livepatch kpatch/patches/shfl-on-qspin.patch
```

2. Start VM and pin vcpu. Or you can run on the host machine with compiled linux
   kernel.

3. Send compiled policy programs(`output` directory) into the VM.

4. Inside the VM, now you can dynamically enable/disable NUMA-grouping policy.
   This repo has two example policy: numa-grouping and shuffling-off.
   numa-grouping groups threads running on a same NUMA socket, so that it works
   exactly same as shfllock.
   shuffling-off policy disables the shuffling mechanisms.

	To enable a policy, first pin eBPF program under BPFFS.
	```
	(VM)$ sudo mount bpffs -t bpf /sys/fs/bpf
	(VM)$ sudo ./eBPFGen/<policy-name>/<policy-name>
	```

	Then you can see the following message.
	```
	./eBPFGen/<policy-name> loaded
	prog loaded well
	```

	Enable the policy by inserting livepatch module.
	```
	(VM)$ sudo insmod ./Livepatch/shfllock/livepatch-concord.ko
	```

Now the policy is enabled.


5. To disable the policy, run the following script.

```
(VM)$ echo 0 > /sys/kernel/livepatch/livepatch_concord/enabled
(VM)$ rm /sys/fs/bpf/*
(VM)$ sudo rmmod livepatch_concord
```

## How to make a policy

With concord, user can create own custom policy in userspace and dynamically
enable/disable it.

You can start from creating a directory under `./concord/policy`. The name of
the directory would be the policy name.

Inside the directory, you need following files:

- `concord.h` : Copy of `./concord/policy/concord.h`. You can modify this
	  file to add auxiliary data per thread node or per lock.
- `<function_name>.pfile` : An simplified eBPF code to create lock policy.
	  You can have multiples of pfile in the directory.

`concord/policy/numa-grouping` directory is a simple example policy working on
top of shfllock. Inside the `numa-grouping.h` file, you can find that the policy
requires `nid` variable per node.

Inside the `numa-grouping.pfile` file, it compares the socket id of current node
and anchor node. The result will be used inside the `shuffle_waiters` of
shfllock whether to move current node forward.

In addition to the `custom_cmp_func()` API used in `numa-grouping.pfile`,
Concord provides several APIs. You can find the list under
`concord/lockdir/list_APIs.h`.
