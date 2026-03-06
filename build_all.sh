#!/bin/env bash

mkdir windows
mkdir web
mkdir linux
mkdir android

just build linux
just build web
just build android
just build windows