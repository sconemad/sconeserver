#! /bin/bash

for file in arg/*.test; do

  echo ""
  echo "################################################"
  echo "  RUNNING ARG TEST SCRIPT: $file"
  echo "################################################"
  echo ""

  ./argtest < $file

done
