#!/bin/bash
while getopts o:m:f: option
do
    case "${option}" in
        o) OutFile=${OPTARG};;
        m) Backend=${OPTARG};;
        f) Frontend=${OPTARG};;
        *) echo "Err"; exit;;
    esac
done
shift $(($OPTIND - 1)) 
if [[ $# -ne 1 ]]; then 
    echo "Bad arguments"
    exit
fi

File=$1
baseName=${File##*/}

if [[ -z "$Frontend" ]]; then
    Frontend=${File##*.}
fi

if [[ -z "$Backend" ]]; then
    Backend="asm"
fi

AsmFile=${baseName%.*}.${Backend}

if [[ -z "$OutFile" ]]; then
    @echo $OutFile
    OutFile=${baseName%.*}.out
fi 

if [[ ! -e ncc/FrontEnd/${Frontend} ]]; then
    echo "Frontend \"${Frontend}\" not found"
    exit
fi

if [[ ! -e ncc/BackEnd/${Backend} ]]; then
    echo "Backend \"${Backend}\" not found"
    exit
fi

tmpFile=${baseName}.tree

./ncc/FrontEnd/${Frontend} "$File" "$tmpFile"
./ncc/MiddleEnd "$tmpFile"
./ncc/BackEnd/${Backend} "$tmpFile" "$AsmFile"
./asm/${Backend} "$AsmFile"

if [[ "${baseName%.*}.out" !=  "$OutFile" ]]; then
    mv "${baseName%.*}.out" "$OutFile" 
fi

rm "$AsmFile" "${baseName%.*}.lst" "$tmpFile"