# Tiger and caretaker

There is a tiger in a cage. The caretaker feeds him meat (a bowl of meat is a *shared memory*). The tiger from time to time eats several kilograms of meat, as much as he accidentally wants. When the bowl is empty, the tiger is kind and does not bite, the caretaker can come in and put in more meat. By pressing <Ctrl-C>, all processes (tiger and caretaker) terminate correctly.

### Load:
```sh
$ git clone https://github.com/Berezniker/IPC.git
$ cd IPC
```

### Compile:
```sh
$ gcc cell.c -o cell
```

### Run:
```sh
$ ./cell
```

<img src="https://github.com/Berezniker/IPC/blob/master/console.png">
