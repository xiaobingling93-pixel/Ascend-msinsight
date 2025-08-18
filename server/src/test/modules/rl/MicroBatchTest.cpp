/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "RLMicroBatchMegatronClassifier.h"
#include "gtest/gtest.h"

using namespace Dic::Module::RL;

class MicroBatchTest : public testing::Test {
protected:
    class MicroBatchMegaTronTest : public RLMicroBatchMegatronClassifier {
    public:
        std::vector<RLPipelineNode> Classifier(std::vector<RLPipelineNode> &items)
        {
            return RLMicroBatchMegatronClassifier::MicroBatchClassifier(items);
        }
    };
    std::vector<RLPipelineNode> generatorOneByOneData()
    {
        std::vector<RLPipelineNode> res;
        RLPipelineNode fp1{"", "FP", 0, 10, "transformerBlock", "rollout"};    // set [0,10]
        res.push_back(fp1);
        RLPipelineNode bp1{"", "BP", 12, 10, "transformerLayer", "rollout"}; // set [12, 22]
        res.push_back(bp1);
        RLPipelineNode fp2{"", "FP", 24, 10, "transformerBlock", "rollout"}; // set [24, 34]
        res.push_back(fp2);
        RLPipelineNode bp2{"", "BP", 35, 2, "transformerLayer", "rollout"};   // set [35 , 37]
        res.push_back(bp2);
        return res;
    }

    std::vector<RLPipelineNode> generatorData1()
    {
        std::vector<RLPipelineNode> res;
        RLPipelineNode fp1{"", "FP", 0, 50, "transformerBlock", "rollout"};   // set [0, 50]
        res.push_back(fp1);
        RLPipelineNode bp1{"", "BP", 1, 2, "transformerLayer", "rollout"};    // set [1, 2]
        res.push_back(bp1);
        RLPipelineNode bp2{"", "BP", 4, 4, "transformerLayer", "rollout"};    // set [4, 4]
        res.push_back(bp2);
        RLPipelineNode fp2{"", "FP", 51, 10, "transformerBlock", "rollout"};  // set [51, 61]
        res.push_back(fp2);
        RLPipelineNode bp3{"", "BP", 62, 2, "transformerLayer", "rollout"};   // set [62, 64]
        res.push_back(bp3);
        RLPipelineNode bp4{"", "BP", 65, 3, "transformerLayer", "rollout"};   // set [65, 68]
        res.push_back(bp4);
        RLPipelineNode bp5{"", "BP", 69, 10, "transformerLayer", "rollout"};  // set [69, 10]
        res.push_back(bp5);
        return res;
    }
};

TEST_F(MicroBatchTest, normalOnebyOne)
{
    auto originalData = generatorOneByOneData();
    MicroBatchMegaTronTest classifier;
    auto res = classifier.Classifier(originalData);
    EXPECT_EQ(res.size(), 4);       // expect size 4
    auto node1 = res[0];
    EXPECT_EQ(node1.nodeType, "FP");
    EXPECT_EQ(node1.startTime, 0);
    EXPECT_EQ(node1.duration, 10);      // expect duration 10
    auto node2 = res[1];                // get 1
    EXPECT_EQ(node2.nodeType, "BP");
    EXPECT_EQ(node2.startTime, 12);     // expect startTime 12
    EXPECT_EQ(node2.duration, 10);      // expect duration 10
    auto node3 = res[2];                // get 2
    EXPECT_EQ(node3.nodeType, "FP");
    EXPECT_EQ(node3.startTime, 24);     // expect startTime 24
    EXPECT_EQ(node3.duration, 10);      // expect duration 10
    auto node4 = res[3];                // get 3
    EXPECT_EQ(node4.nodeType, "BP");
    EXPECT_EQ(node4.startTime, 35);     // expect start time 35
    EXPECT_EQ(node4.duration, 2);       // expect duration 2
}

TEST_F(MicroBatchTest, timeCover)
{
    auto original = generatorData1();
    MicroBatchMegaTronTest classifier;
    auto res = classifier.Classifier(original);
    EXPECT_EQ(res.size(), 4);  // expect size 4
    auto node1 = res[0];
    EXPECT_EQ(node1.nodeType, "FP");
    EXPECT_EQ(node1.startTime, 0);
    EXPECT_EQ(node1.duration, 50);      // expect duration is 50ns
    auto node2 = res[1];
    EXPECT_EQ(node2.nodeType, "FP");
    EXPECT_EQ(node2.startTime, 51);     // expect start time 51ns
    EXPECT_EQ(node2.duration, 10);      // expect duration 10ns
    auto node3 = res[2];                // get 2
    EXPECT_EQ(node3.nodeType, "BP");
    EXPECT_EQ(node3.startTime, 62);     // expect start time is 62ns
    EXPECT_EQ(node3.duration, 6);       // expect duration time 6ns
    auto node4 = res[3];                // get 3
    EXPECT_EQ(node4.nodeType, "BP");
    EXPECT_EQ(node4.startTime, 69);     // expect start time 69
    EXPECT_EQ(node4.duration, 10);      // expect duration 10
}
