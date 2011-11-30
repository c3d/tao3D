# Usage sign_temporary.sh <expiry date> files...
# Files must end in .notsigned
# Example: sign_temporary.sh 15-Mar-2012 license.taokey.notsigned

BASEDIR=$(dirname $0)
DATE=$1
shift
while [ "$1" != "" ]; do
  NOTSIGNED=$1
  SIGNED=`echo $1 | sed 's/\.notsigned$//'`
  if [ "$SIGNED" = "$NOTSIGNED" ]; then
    echo $0: $1: file does not have the .notsigned suffix, skipped
  else
    echo "expires $DATE" > $SIGNED
    cat $NOTSIGNED >> $SIGNED
    $BASEDIR/tao_sign.sh $SIGNED || (rm $SIGNED ; echo $0: Error signing $SIGNED)
  fi
  shift
done
