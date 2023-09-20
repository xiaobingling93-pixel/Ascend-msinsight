if [ -n "$ENV_MR_INFO" ]; then
cherryPick=cherry-pick
releaseBranch=IDE_Release_
weeklyBranch=IDE_weekly_
betaBranch=IDE_Beta_
merge_id=`echo $ENV_MR_INFO | grep -Po '(?<="mrIid":")[^"]*'`
target_branch=`echo $ENV_MR_INFO | grep -Po '(?<="targetBranch":")[^"]*'`
source_branch=`echo $ENV_MR_INFO | grep -Po '(?<="sourceBranch":")[^"]*'`
if [[ $target_branch != ${releaseBranch}*  &&   $target_branch != ${weeklyBranch}*  &&   $target_branch != ${betaBranch}*   ]]
then
echo " merge into  $target_branch ,continue..."
elif [[ $source_branch =~ $cherryPick ]]
then
echo " cherry-picked to $target_branch ,continue..."
else
echo "ERROR! ============please merge into master first,then cherry-pick to  $target_branch !!!===================="
exit 1
fi
fi
set -e
cd ohos-hiinsight

chmod +x gradlew
java --version

cd $WORKSPACE/ohos-hiinsight
./gradlew buildLibrary --stacktrace -PLIBRARY_VERSION=$1 -PRELEASE_VERSION=$2 -PBINARY_VERSION=$3 -Pinternal=$4 -PUSERNAME=$5 -PPASSWORD=$6
