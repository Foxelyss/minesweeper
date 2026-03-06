#!/bin/env bash

zip linux.zip linux/*
zip android.zip android/*
zip windows.zip windows/*
cd web && zip ../web.zip *
cd ..