# aadl-eventdata-direct-new (VM components)

This modified camkes app (aadl-eventdata-direct-new) demonstrates AADL eventdata
connectors between native camkes components and guest VMs. Starting with the baseline
scenario of a native sender and native receiver, each of the sender or receiver could
be replaced with a VM component. The combination of substitutions gives 4 scenarios.
See at the bottom for checking out the sources.

There are 2 VM guest applications, `receiver` and `sender`. They can be started from
the shell and require an argument to the device file corresponding to the VM connector,
this is `/dev/uio0`, followed by an argument for the size of the dataport, `4096`.
The implementations of these apps then use the queue interface and file descriptor
to interact with the shared memory and signalling mechanisms in a similar way to the
native applications.  In the following sections, there is a snippet of the camkes
composition describing the scenario, followed by expected console output from
running the applications on an odroid-xu4. The VM scenarios require interaction on
the console to start the Linux applications once the VMs have booted.

Switching between the scenarios should be done by checking out the commit for each scenario as
there are domain configuration changes as well as camkes `composition` changes for
some scenarios:

```
commit 34d048447ce858d6283542636888f6fbf3f07ed7
Author: Kent McLeod <Kent.Mcleod@data61.csiro.au>
Date:   Mon Feb 24 17:25:11 2020 +1100

    scenario 4

commit a41a9707c12c4e00886f05bcb97d146d14b1ee0d
Author: Kent McLeod <Kent.Mcleod@data61.csiro.au>
Date:   Mon Feb 24 17:23:43 2020 +1100

    scenario 3

commit 4958412c88ab717aa0a37d4ca3bb4b76d3207a62
Author: Kent McLeod <Kent.Mcleod@data61.csiro.au>
Date:   Mon Feb 24 17:23:24 2020 +1100

    Scenario 2

commit 949bd36a1ee9dda447b148df68ad88be78646d0a
Author: Kent McLeod <Kent.Mcleod@data61.csiro.au>
Date:   Mon Feb 24 17:22:33 2020 +1100

    scenario 1

```

## Scenario 1: Native sender, Native receiver

```
    component Receiver receiver;
    component Sender sender;

    // AADL Event Data Port connection representation from sender.p1_out to reciever.p1_in
    connection seL4Notification pc1_event(from sender.p1_out_SendEvent, to receiver.p1_in_SendEvent);
    connection seL4SharedData pc1_queue(from sender.p1_out_queue, to receiver.p1_in_queue);
```


```
  virt_entry=e0000000
ELF-loading image 'capdl-loader'
  paddr=[6004e000..60216fff]
  vaddr=[10000..1d8fff]
  virt_entry=17c44
Enabling hypervisor MMU and paging
Jumping to kernel-image entry point...

Bootstrapping kernel
Booting all finished, dropped to user space
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothsender: sending
ing: 1
sender: sending: 2
sender: sending: 3
sender: sending: 4
sender: sending: 5
sender: sending: 6
sender: sending: 7
receiver: received: 1  numDropped: 0
receiver: received: 2  numDropped: 0
receiver: received: 3  numDropped: 0
receiver: received: 4  numDropped: 0
receiver: received: 5  numDropped: 0

```
## Scenario 2: Native sender, VM receiver
```
    // Scenario 2: Nativer sender, VM receiver
    component Sender sender;
    VM_GENERAL_COMPOSITION_DEF()
    component VM vmReceiver;
    VM_COMPONENT_CONNECTIONS_DEF(Receiver)
    /* vm serial connections */
    VM_VIRTUAL_SERIAL_COMPOSITION_DEF(Receiver)

    connection seL4GlobalAsynch event_conn_1(from sender.p1_out_SendEvent, to vmReceiver.done);
    connection seL4SharedDataWithCaps cross_vm_conn_1(from sender.p1_out_queue, to vmReceiver.crossvm_dp_0);
    connection seL4VMDTBPassthrough vm_dtb1(from vmReceiver.dtb_self, to vmReceiver.dtb);


```

To start the receiver on the VM side, the `receiver` program (located in `vm/apps/receiver`)
needs to be called: `receiver /dev/uio0 4096` to attach to the receiving end of the connectors.
```
Welcome to Buildroot
buildroot login: root
# receiver /dev/uio0 4096
sender: sending: 2831
sender: sending: 2832
sender: sending: 2833
sender: sending: 2834
sender: sending: 2835
sender: sending: 2836
sender: sending: 2837
sender: sending: 2838
sender: sending: 2839
sender: sending: 2840
sender: sending: 2841

Receiver: received: 2835  numDropped: 2834
Receiver: received: 2836  numDropped: 0
Receiver: received: 2837  numDropped: 0
Receiver: received: 2838  numDropped:sender: 0
 sending: 2842
sender: sending: 2843
sender: sending: 2844
sender: sending: 2845
sender: sending: 2846
sender: sending: 2847
Receiver: received: 2841  numDropped: 2
Receiver: received: 2842  numDropped: 0
Receiver: received: 2843  numDropped: 0
Receiver: received: 2844  numDropped: 0
Receiver: received: 2845  numDropped: 0
sender: sending: 2848
sender: sending: 2849
sender: sendinReceiver: received: 2846  numDroppedg: 2850
: 0
sender: sending: 2851
Receiver: received: 2847  numDropped: 0

```

## Scenario 3: VM sender, Native receiver

```
    // Scenario 3: VM Sender, Native receiver
    component Receiver receiver;
    VM_GENERAL_COMPOSITION_DEF()
    component VM vmSender;
    VM_COMPONENT_CONNECTIONS_DEF(Sender)
    /* vm serial connections */
    VM_VIRTUAL_SERIAL_COMPOSITION_DEF(Sender)

    connection seL4Notification event_conn_1(from vmSender.ready, to receiver.p1_in_SendEvent);
    connection seL4SharedDataWithCaps cross_vm_conn_1(from vmSender.crossvm_dp_0,  to receiver.p1_in_queue);
    connection seL4VMDTBPassthrough vm_dtb1(from vmSender.dtb_self, to vmSender.dtb);
```

The sender in the VM needs to be started by running `sender /dev/uio0 4096`. This calls the
sender binary located in `vm/apps/sender`.
```
[    4.075787] connector 0000:00:02.0: assign IRQ: got 21
[    4.080868] Event Bar (dev-0) initalised
[    4.087914] 2 Dataports (dev-0) initalised
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing

Welcome to Buildroot
buildroot login: receiver: received nothing
receiver: received nothing
receiver: received nothing

# sender /dev/uio0 4096
receiver: received nothing
receiver: received nothing
sender: sending: 1
sender: sending: 2
sender: sending: 3
receiver: received: 1  numDropped: 0
receiver: received: 2  numDropped: 0
receiver: received: 3  numDropped: 0
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
receiver: received nothing
recsender: sending: 4
sender: sendingeiver: re: 5
sender: sending: 6
sender: sceived noending: 7
sender: sending: 8
thing
receisender: sending: 9
ver: received: 4  numDropped: 0
receiver: received: 5  numDropped: 0
receiver: received: 6  numDropped: 0
receiver: received: 7  numDropped: 0
receiver: received: 8  numDropped: 0
receiver: received: 9  numDropped: 0
receiver: received nothing
receiver: received nothing
receiver: received nothing
```
## Scenario 4: VM sender, VM receiver

The `seL4GlobalAsynch` connector is used to allow the VM to receive signals. Because the VM component only has
one thread that can be used to receive events, a custom connector is used.

```
    // Scenario 4: VM Sender and VM receiver
    VM_GENERAL_COMPOSITION_DEF()

    component VM vmReceiver;
    VM_COMPONENT_CONNECTIONS_DEF(Receiver)
    component VM vmSender;
    VM_COMPONENT_CONNECTIONS_DEF(Sender)
    /* vm serial connections */
    VM_VIRTUAL_SERIAL_COMPOSITION_DEF(Receiver,Sender)

    connection seL4GlobalAsynch event_conn_1(from vmSender.ready, to vmReceiver.done);
    connection seL4SharedDataWithCaps cross_vm_conn_0(from vmSender.crossvm_dp_0, to vmReceiver.crossvm_dp_0);
    connection seL4VMDTBPassthrough vm_dtb(from vmReceiver.dtb_self, to vmReceiver.dtb);
    connection seL4VMDTBPassthrough vm_dtb1(from vmSender.dtb_self, to vmSender.dtb);
```

When using 2 VMs, the serial server is used to switch input between each VM. The serial server has an escape
character `@` for switching inputs. Switching to client 1 from client 0 would be by `@1`.
```
 --- SerialServer help ---
 Escape char: @
 0 - 1  switches input to that client
 ?      shows this help
 m      simultaneous multi-client input
 d      switch between debugging modes
          0: no debugging
          1: debug multi-input mode output coalescing
          2: debug flush_buffer_line
```

To start the receiver:
```
Welcome to Buildroot
buildroot login: root
# receiver /dev/uio0 4096
```

To start the sender:
```
Welcome to Buildroot
buildroot login: root
# sender /dev/uio0 4096
sender: sending: 1
sender: sending: 2
sender: sending: 3
Receiver: received: 1  numDropped: 0
Receiver: received: 2  numDropped: 0
Receiver: received: 3  numDropped: 0
sender: sending: 4
sender: sending: 5
sender: sending: 6
sender: sending: 7
sender: sending: 8
sender: sending: 9
Receiver: received: 4  numDropped: 0
```

## Checkout the sources

```
repo init -u https://github.com/SEL4PROJ/camkes-arm-vm-manifest.git --depth=1
repo sync -j8
(cd projects && git clone https://github.com/kent-mcleod/camkes.git -b kent/aadl)
ln -sf projects/camkes/easy-settings.cmake
mkdir build
cd build
../init-build.sh -DPLATFORM=exynos5422 -DCAMKES_APP=aadl-eventdata-direct-new -DARM_HYP=ON -DARM=ON
ninja

```
