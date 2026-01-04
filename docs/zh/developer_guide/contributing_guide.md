# Contributing
## 构建与测试
在提交PR之前，建议您先在本地搭建开发环境，构建`insight`并运行相关测试。具体请参考：
[搭建开发及测试环境](../developer_guide/development_guide.md)

## PR标题与分类
只有特定类型的PR才会被审核。请在PR标题前添加合适的前缀，以明确PR类型。请使用以下分类之一:

- `[Platform]`: 底座平台相关的新功能、优化或bug修复。
- `[Common]`: 公共模块相关的新功能、优化或bug修复。
- `[Timeline]`: 系统调优-集群相关的新功能、优化或bug修复。
- `[Memory]`: 系统调优-内存相关的新功能、优化或bug修复。
- `[Operator]`: 系统调优-算子相关的新功能、优化或bug修复。
- `[MemScope]`: 系统调优-内存详情相关的新功能、优化或bug修复。
- `[Cluster]`: 系统调优-集群详情相关的新功能、优化或bug修复。
- `[RL]`: 系统调优-强化学习相关的新功能、优化或bug修复。
- `[Advisor]`: 系统调优-专家建议相关的新功能、优化或bug修复。
- `[Source]`: 算子调优相关的新功能、优化或bug修复。
- `[Servitization]`: 服务化调优相关的新功能、优化或bug修复。

## Commit Requirement
为保持commit记录清晰，请确保每个PR仅包含一个commit。
如果您的PR当前包含多个commits，请在提交前使用以下任一方法（包括但不限于）将其合并为单个commit。(尽管GitCode在合并PR时提供了`Squash 合并`的选项, 提前将PR整理为单个简洁的commit仍然被视为最佳实践，并且深受committer们的欢迎。)
### 方式一：交互式变基（推荐）
- 查看需要合并的最近几个commit（例如最近3个）：
``` bash
git log --oneline -n 3
```
- 启动交互式rebase (将`N`替换为需要合并的commit数量):
``` bash
git rebase -i HEAD~N
```
- 在打开的编辑器中:
    - 保留第一个commit的`pick`。
    - 将其余commit前的`pick`修改为`squash`(或简写为`s`) 。
- 保存并关闭。随后会打开新窗口，供您编写合并后的简洁、有意义的commit信息。
- 强制推送更新后的分支 (仅限于您自己的特性分支):
``` bash
git push --force-with-lease origin your-branch-name
```
### 方式二：reset + 新建commit
```bash
# 获取最新的待合入的目标分支（如main）
git fetch origin main

# Soft-reset到主干分支--此操作会保存所有修改并回归到暂存区。
git reset --soft origin/main

# 将所有更改提交为一个新的commit
git commit -m "feat: concise description of your change"

# 强制推送以更新PR分支
git push --force-with-lease origin your-branch-name

```
> 提示: 如果您不确定应基于哪个目标分支，请查看仓库的默认分支或咨询Maintainer.

> 警告：切勿对共享或受保护的分支执行强制推送。

