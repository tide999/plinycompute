/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#include <AdvancedPhysicalOptimizer/Algorithms/AdvancedPhysicalShuffleSetAlgorithm.h>
#include <JobStageBuilders/TupleSetJobStageBuilder.h>

namespace pdb {

AdvancedPhysicalShuffleSetAlgorithm::AdvancedPhysicalShuffleSetAlgorithm(const AdvancedPhysicalPipelineNodePtr &handle,
                                                                         const std::string &jobID,
                                                                         bool isProbing,
                                                                         bool isOutput,
                                                                         const Handle<SetIdentifier> &source,
                                                                         const Handle<ComputePlan> &computePlan,
                                                                         const LogicalPlanPtr &logicalPlan,
                                                                         const ConfigurationPtr &conf) : AdvancedPhysicalAbstractAlgorithm(handle,
                                                                                                                                           jobID,
                                                                                                                                           isProbing,
                                                                                                                                           isOutput,
                                                                                                                                           source,
                                                                                                                                           computePlan,
                                                                                                                                           logicalPlan,
                                                                                                                                           conf) {}
PhysicalOptimizerResultPtr AdvancedPhysicalShuffleSetAlgorithm::generate(int nextStageID,
                                                                         const StatisticsPtr &stats) {

  // extract the atomic computations from the pipes for this algorithm
  extractAtomicComputations();

  // extract the hash sets we might want to probe
  extractHashSetsToProbe();

  // get the source atomic computation
  auto sourceAtomicComputation = this->pipelineComputations.front();

  // we get the first atomic computation of the join pipeline this should be the apply join computation
  auto joinAtomicComputation = pipeline.back()->getConsumer(0)->to<AdvancedPhysicalAbstractPipe>()->getPipelineComputationAt(0);

  // get the final atomic computation
  string finalAtomicComputationName = this->pipelineComputations.back()->getOutputName();

  // the computation specifier of this join
  std::string computationSpecifier = joinAtomicComputation->getComputationName();

  // grab the output of the current node
  std::string outputName = joinAtomicComputation->getOutputName();

  // grab the computation associated with this node
  Handle<Computation> curComp = logicalPlan->getNode(computationSpecifier).getComputationHandle();

  // fist we need to shuffle our data from the other side and put it in this sink set
  sink = makeObject<SetIdentifier>(jobID, outputName + "_repartitionData");
  sink->setPageSize(conf->getBroadcastPageSize());

  // create a tuple set job stage builder
  TupleSetJobStageBuilderPtr tupleStageBuilder = make_shared<TupleSetJobStageBuilder>();

  // copy the computation names
  for(const auto &it : this->pipelineComputations) {

    // we don't need the output set name... (that is jsut the way the pipeline building works)
    if(it->getAtomicComputationTypeID() == WriteSetTypeID) {
      continue;
    }

    // add the set name of the atomic computation to the pipeline
    tupleStageBuilder->addTupleSetToBuildPipeline(it->getOutputName());
  }

  // set the parameters
  tupleStageBuilder->setSourceTupleSetName(sourceAtomicComputation->getOutputName());
  tupleStageBuilder->setSourceContext(source);
  tupleStageBuilder->setInputAggHashOut(source->isAggregationResult());
  tupleStageBuilder->setJobId(jobID);
  tupleStageBuilder->setProbing(isProbing);
  tupleStageBuilder->setComputePlan(computePlan);
  tupleStageBuilder->setJobStageId(nextStageID++);
  tupleStageBuilder->setTargetTupleSetName(finalAtomicComputationName);
  tupleStageBuilder->setTargetComputationName(computationSpecifier);
  tupleStageBuilder->setOutputTypeName("IntermediateData");
  tupleStageBuilder->setSinkContext(sink);
  tupleStageBuilder->setRepartition(true);
  tupleStageBuilder->setAllocatorPolicy(curComp->getAllocatorPolicy());
  tupleStageBuilder->setRepartitionJoin(true);

  // add all the probing hash sets
  for(auto it : probingHashSets) {
    tupleStageBuilder->addHashSetToProbe(it.first, it.second);
  }

  // update the consumers
  updateConsumers(sink, approximateResultSize(stats), stats);

  // we first create a pipeline breaker to partition RHS by setting
  // the isRepartitioning=true and isRepartitionJoin=true
  Handle<TupleSetJobStage> joinPrepStage = tupleStageBuilder->build();

  // create a analyzer result
  PhysicalOptimizerResultPtr result = make_shared<PhysicalOptimizerResult>();

  // add the stage to the list of stages to be executed
  result->physicalPlanToOutput.emplace_front(joinPrepStage);

  // add the output of this TupleSetJobStage to the list of intermediate sets
  result->interGlobalSets.emplace_front(sink);

  // set the remaining parameters of the result
  result->success = true;

  // update the source pipes to reflect the state after executing the job stages
  // if we have a consumer we have a new source pipe since we materialize this result
  if(pipeline.back()->getNumConsumers() != 0) {

    // add consumers as new sources
    for(int i = 0; i < pipeline.back()->getNumConsumers(); ++i) {
      result->createdSourceComputations.push_back(pipeline.back()->getConsumer(i));
    }
  }

  return result;
}

AdvancedPhysicalAbstractAlgorithmTypeID AdvancedPhysicalShuffleSetAlgorithm::getType() {
  return JOIN_SUFFLE_SET_ALGORITHM;
}

}


