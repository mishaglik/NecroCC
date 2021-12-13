#!/bin/bash
while getopts o:m:f:r option
do
    case "${option}" in
        o) OutFile=${OPTARG};;
        m) Backend=${OPTARG};;
        f) Frontend=${OPTARG};;
        r) Reversed="1";;
        *) echo "Err"; exit;;
    esac
done
shift $(($OPTIND - 1)) 
if [[ $# -lt 1 ]]; then 
    echo "Bad arguments"
    exit
fi

File=$1
baseName=${File##*/}

if [[ -n "$Reversed" ]]; then
    if [[ -z "$Frontend" ]]; then
        Frontend=${OutFile##*.}
    fi
    if [[ -z "$Frontend" ]]; then
        Frontend="cht"
    fi
    if [[ -z "$OutFile" ]]; then
        OutFile=${baseName%.*}.${Frontend}
    fi

    ./ncc/FrontEnd/${Frontend} - "$File" "$OutFile"
    exit;
fi

if [[ -z "$Frontend" ]]; then
    Frontend=${File##*.}
fi

if [[ -z "$Backend" ]]; then
    Backend="asm"
fi

AsmFile=${baseName%.*}.${Backend}

if [[ -z "$OutFile" ]]; then
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
if [[ ! -e "$tmpFile" ]]; then
    exit
fi
./ncc/MiddleEnd "$tmpFile"
./ncc/BackEnd/${Backend} "$tmpFile" "$AsmFile"
if [[ ! -e "$AsmFile" ]]; then 
    exit
fi
./asm/${Backend} "$AsmFile"

if [[ "${baseName%.*}.out" !=  "$OutFile" ]]; then
    mv "${baseName%.*}.out" "$OutFile" 
fi

rm "$AsmFile" "${baseName%.*}.lst" "$tmpFile"
