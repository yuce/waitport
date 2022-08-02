# waitport

`waitport` is a simple utility that waits for a TCP port to be connectable.
It is very small, about 10KBs when compiled and stripped.

## Build

Requirements:

* Linux (May work on other POSIX).
* C compiler with C99 support.

### Using Make

Just:
```
$ make
```

### Using CMake

```
$ mkdir build && cd $_
$ cmake ..
$ make
```

## Usage

Wait indefinitely on port 9701 at localhost, try every second:
```
$ waitport 9701
```

Wait indefinitely on port 9701 at foobar.com, try every second:
```
$ waitport foobar.com 9701 
```

Wait 10 seconds on port 9701, try every second:
```
$ waitport 9701 -t 10
```

Wait 10 seconds on port 9701, try every 100 ms:
```
$ waitport 9701 -t 10 -s 0.1
```

### License

[MIT](LICENSE)

