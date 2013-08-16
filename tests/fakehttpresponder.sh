#!/bin/bash

if [ $# -lt 1 ]; then
    echo "usage: $0 <data folder> [slow] [rate]"
    echo
    echo "data folder: should be equivalent with a public folder shared over http"
    echo ""
    echo "slow parameter can be used to simulate a slow transfer"
    echo "rate parameter is the size (in bytes) of a block to transfer during a slow transfer simulation"
    echo
    echo "hint:  use script with socat to act as a http server. the script"
    echo "       will listen for requests on stdin and send a responses on stdout"
    echo
    echo "example: socat TCP4-LISTEN:8080,fork,tcpwrap=script EXEC:\"$0 testdata \""
    echo
    echo "         test with: telnet localhost 8080"
    echo
    exit 1
fi

VERBOSE=1
SLOWTRANSFER="false"
SLOWTRANSFERRATE="2048"

debug()
{
    if [ $VERBOSE = 1 ]; then
        echo "$1" >&2
    fi
}

if [ $# -ge 2 ] && [ $2 = "slow" ]; then
    SLOWTRANSFER="true"
fi

if [ $# -ge 3 ] && [ $2 = "slow" ]; then
    SLOWTRANSFERRATE=$3
fi

debug ""
debug "PID:$$ Waiting for request"

getrequest=""
read -r -t 1
while [ $? -eq 0 ] && [ ${#REPLY} -ne 1 ]; do
    debug "PID:$$ Request: $REPLY"
	# Store the GET request if we get one
	case $REPLY in
	GET*)
		getrequest="$(echo $REPLY | cut -d ' ' -f 2 | tr -d "\r\n/\" ")"
		;;
	esac
	HEADERS="${HEADERS}\r\n${REPLY}"
	read -r -t 1
done

debug
debug "PID:$$ File '$getrequest' was requested"

date=$(date -R)
ErrorResponse="HTTP/1.1 404 Not Found
Date: ${date}
Server: Apache/2.2.9 (Debian) DAV/2 SVN/1.5.1 PHP/5.2.6-1+lenny9 with Suhosin-Patch mod_ssl/2.2.9 OpenSSL/0.9
Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0
Pragma: no-cache
Content-Length: 199
Connection: close
Content-Type: text/html; charset=utf-8

<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">
<html>
<head>
<title>404 Not Found</title>
</head>
<body>
<h1>Not Found</h1>
<p>The requested URL was not found on this server.</p>
</body>
</html>"

# find request file
datafolder="$1"
if [ ! -f  "$datafolder/$getrequest" ]; then
    debug "PID:$$ Requested file '$getrequest' not found in data folder '$datafolder', returning 404"
    echo "${ErrorResponse}"
    exit
fi

#debug "PID:$$ Reading $contentlength bytes of request body $REPLY"
# read body char by char
#read -r -n 1 -t 1
#while [ $? -eq 0 ] && [ $contentlength -gt 1 ]; do
#	BODY="${BODY}${REPLY}"
#	let contentlength=($contentlength - 1)
#        read -r -n 1 -t 1
#done
#debug "PID:$$ Request-body: $BODY"
#
#debug ""
#debug "PID:$$ Request for \"$soapaction\" complete"
#
#if [ "$soapaction" == "" ]; then
#    debug "PID:$$ Empty soapaction, cannot respond"
#	exit
#fi


file_extension=$(echo ${getrequest#*.})
content_type=""
case $file_extension in
    XML)
        content_type="text/xml; charset=utf-8"
        ;;
    xml)
        content_type="text/xml; charset=utf-8"
        ;;
    HTML)
        content_type="text/html; charset=utf-8"
        ;;
    html)
        content_type="text/html; charset=utf-8"
        ;;
esac
content_length=$(stat --format=%s $datafolder/$getrequest)

debug "PID:$$ Sending response from file '$getrequest'"

if [ $SLOWTRANSFER = "true" ]; then
    timeout=5
    while [ $timeout -gt 0 ]; do
        debug "PID:$$ waiting $timeout second(s) before sending reponse"
        timeout=$(($timeout - 1))
        sleep 1
    done
fi

ResponseData=$(cat $datafolder/$getrequest)
ResponseHeader="HTTP/1.1 200 Ok
Date: $date
Server: Apache/2.2.9 (Debian) DAV/2 SVN/1.5.1 PHP/5.2.6-1+lenny9 with Suhosin-Patch mod_ssl/2.2.9 OpenSSL/0.9
Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0
Pragma: no-cache
Content-Length: $content_length
Connection: keep-alive
Content-Type: $content_type"

debug "PID:$$ $ResponseHeader"
debug ""
#debug "$ResponseData"

echo "${ResponseHeader}"
echo ""
if [ $SLOWTRANSFER = "true" ]; then
    # read SLOWTRANSFERRATE bytes at a time and send it, then sleep 0.5 seconds
    while read -n $SLOWTRANSFERRATE block; do
        echo "$block"
        sleep 0.5
    done < $datafolder/$getrequest
else
    echo "${ResponseData}"
fi
