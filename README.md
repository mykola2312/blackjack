# blackjack

Software to inject .so libraries into running process

For Linux and FreeBSD

## Compnents

blackjack consists of multiple components that are responsible for different tasks.

### blackjack

Hijacks runtime process in order to inject shared objects.

### rtdisasm

KISS robust runtime "disassembler". Used to analyze instructions encoded sizes and find desired instructions for trampolines. No need to bloat it with full-blown disassembler logic like other projects do - one big lookup table is enough for such purposes.
