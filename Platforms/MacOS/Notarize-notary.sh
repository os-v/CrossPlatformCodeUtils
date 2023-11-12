#
#  @file Notarize.sh
#  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
#  @see https://github.com/os-v/CrossPlatformCodeUtils/
#
#  Created on 12.11.23.
#  Copyright 2023 Sergii Oryshchenko. All rights reserved.
#

CurDir=$(cd "$(dirname "$0")"; pwd)

AppPath=$1
ProductName=$2
CertName=$3
DevIDUser=$4
DevIDPass=$5
DevIDProv=$6
ProfileName="DeveloperProfile $DevIDProv"

AppFile="${AppPath##*/}"
AppBase="${AppPath%/*}" 

echo "Signing ..."

codesign --deep --force --verify --verbose --options runtime --sign "$CertName" "$AppPath"

if [ "${AppFile##*.}" = $AppFile ]; then
	AppPath=./${AppFile}.zip
	ditto -ck --rsrc --sequesterRsrc $AppFile $AppPath
fi
   
echo "Uploading, please wait ..."

xcrun notarytool store-credentials --apple-id "$DevIDUser" --password "$DevIDPass" --team-id "$DevIDProv" "$ProfileName"

xcrun notarytool submit "$AppPath" --keychain-profile "$ProfileName" --wait

xcrun stapler staple "$AppPath"


