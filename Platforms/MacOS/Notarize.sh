#
#  @file Notarize.sh
#  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
#  @see https://github.com/os-v/CrossPlatformCodeUtils/
#
#  Created on 31.03.22.
#  Copyright 2022 Sergii Oryshchenko. All rights reserved.
#

CurDir=$(cd "$(dirname "$0")"; pwd)

AppPath=$1
ProductName=$2
CertName=$3
DevIDUser=$4
DevIDPass=$5
DevIDProv=$6
RequestTimeout=$7

BundleID="$ProductName"

AppFile="${AppPath##*/}"
AppBase="${AppPath%/*}"

if [ -z $RequestTimeout ]; then
	RequestTimeout=60
fi

echo "Signing ..."

codesign --deep --force --verify --verbose --options runtime --sign "$CertName" "$AppPath"

if [ "${AppFile##*.}" = "app" ]; then
	AppPath=./${AppFile}.zip
	ditto -ck --rsrc --sequesterRsrc --keepParent $AppFile $AppPath
elif [ "${AppFile##*.}" = $AppFile ]; then
	AppPath=./${AppFile}.zip
	ditto -ck --rsrc --sequesterRsrc $AppFile $AppPath
fi

echo "Uploading, please wait ..."

UploadResult=$(xcrun altool --notarize-app --primary-bundle-id "$BundleID" --username "$DevIDUser" --password "$DevIDPass" --asc-provider "$DevIDProv" --file "$AppPath" 2>&1)
UploadReturn=$?

echo "$UploadResult"

if [ $UploadReturn -ne 0 ]; then
	echo "Failed to upload, error: $UploadReturn"
	exit $UploadReturn
fi

RequestUUID=$(echo "$UploadResult" | grep "RequestUUID" | sed -n "s/RequestUUID = *\(.*\)/\1/p")

echo "RequestUUID is $RequestUUID"

if [ -z $RequestUUID ]; then
	echo "Invalid RequestUUID"
	exit -1
fi

for ((iAttempt=1; ; iAttempt++))
do
	echo "Requesting notarization result. Attempt #$iAttempt"
	RequestResult=$(xcrun altool --notarization-info "$RequestUUID" -u "$DevIDUser" -p "$DevIDPass" 2>&1)
	RequestReturn=$?
	Status=$(echo "$RequestResult" | grep "Status:" | sed -n "s/[[:space:]]*Status: *\(.*\)/\1/p")
	StatusCode=$(echo "$RequestResult" | grep "Status Code:" | sed -n "s/[[:space:]]*Status Code: *\(.*\)/\1/p")
	echo "   Status=$Status, Status Code=$StatusCode"
	if [ $RequestReturn -ne 0 ]; then
		echo "$RequestResult"
		echo "Request failed, error: $RequestReturn"
		exit $RequestReturn
	fi
	if [ "$Status" == "invalid" ]; then
		echo "FullMessage: $RequestResult"
		exit -1
	fi
	if [ "$Status" == "success" ] && [ $StatusCode -eq 0 ]; then
		break
	fi
	echo "   Suspending for $RequestTimeout"
	sleep $RequestTimeout
done

echo "Notarization succeeded, stapling ..."

if [[ "${AppFile##*.}" = "app" || "${AppFile##*.}" = $AppFile ]]; then
	echo "Stapling app ..."
	xcrun stapler staple "$AppFile"
	spctl --assess --verbose "$AppFile" 
	ditto -ck --rsrc --sequesterRsrc --keepParent $AppFile $AppPath
else
	echo "Stapling bundle ..."
	xcrun stapler staple "$AppPath"
fi

echo "Notarization finished"

