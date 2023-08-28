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

cd ohos-data-insight-core/build/
mkdir bin
cd bin

rm -rf server
mkdir server
cd server
mkdir mac
mkdir win

cd ../
curl -Lo "PAP_DIC_win.tar.gz" -k https://cmc-szver-artifactory.cmc.tools.huawei.com/artifactory/sz-software-snapshot/deveco%20studio1674895384901/snapshot/Ohos-data-insight-core/1.0.8_win/PAP_DIC_win.tar.gz
curl -Lo "PAP_DIC_mac.tar.gz" -k https://cmc-szver-artifactory.cmc.tools.huawei.com/artifactory/sz-software-snapshot/deveco%20studio1674895384901/snapshot/Ohos-data-insight-core/1.0.8_mac/PAP_DIC_mac.tar.gz

mkdir win
mkdir mac
tar -zxvf PAP_DIC_win.tar.gz -C win
if [ $? -ne 0 ]; then
  echo "failed to tar -zxvf PAP_DIC_win.tar.gz -C win"
  exit 1
else
  echo "tar -zxvf PAP_DIC_win.tar.gz -C win"
fi
tar -zxvf PAP_DIC_mac.tar.gz -C mac
if [ $? -ne 0 ]; then
  echo "failed to tar -zxvf PAP_DIC_mac.tar.gz -C mac"
  exit 1
else
  echo "tar -zxvf PAP_DIC_mac.tar.gz -C mac"
fi

cp win/bin/*.exe server/win
if [ $? -ne 0 ]; then
  echo "failed to cp win/bin/*.exe server/win"
  exit 1
else
  echo "cp win/bin/*.exe server/win"
fi
cp mac/bin/* server/mac
if [ $? -ne 0 ]; then
  echo "failed to cp mac/bin/* server/mac"
  exit 1
else
  echo "cp mac/bin/* server/mac"
fi

tar -zcvf ohos-data-insight-core.tar.gz server/