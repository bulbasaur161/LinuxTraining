# Mechanism
Userspace <-> glibC library(malloc/free) <-> Kernel MMU
- Userspace can use malloc to request glibC provided any of byte: malloc(X bytes)
- glibC use sbrk function to request Kernel MMU provided units of Page sizes(1 Page sizes = )
