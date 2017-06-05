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
#ifndef PART_H
#define PART_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"

// This class represents a Part Object according to the TPC-H Database Benchmark


//CREATE TABLE PART  ( P_PARTKEY     INTEGER NOT NULL,
//        P_NAME        VARCHAR(55) NOT NULL,
//        P_MFGR        CHAR(25) NOT NULL,
//        P_BRAND       CHAR(10) NOT NULL,
//        P_TYPE        VARCHAR(25) NOT NULL,
//        P_SIZE        INTEGER NOT NULL,
//        P_CONTAINER   CHAR(10) NOT NULL,
//        P_RETAILPRICE DECIMAL(15,2) NOT NULL,
//        P_COMMENT     VARCHAR(23) NOT NULL );


class Part:public pdb::Object{

	int partKey;
	pdb::Handle<pdb::String> name;
	pdb::Handle<pdb::String> mfgr;
	pdb::Handle<pdb::String> brand;
	pdb::Handle<pdb::String> type;
	int size;
	pdb::Handle<pdb::String> container;
	double retailPrice;
	pdb::Handle<pdb::String> comment;

public:
	ENABLE_DEEP_COPY

	//Default constructor:
	Part(){}

	//Default destructor:
	~Part() {}

	//Constructor with arguments:
	Part(int partKey, std::string name, std::string  mfgr, std::string  brand,
			std::string  type, int size, std::string  container, double retailPrice,
			std::string  comment) {
		this->partKey= partKey;
		this->name= pdb::makeObject<pdb::String>(name);
		this->mfgr= pdb::makeObject<pdb::String>(mfgr);
		this->brand=pdb::makeObject<pdb::String>(brand);
		this->type=pdb::makeObject<pdb::String>(type);
		this->size=size;
		this->container= pdb::makeObject<pdb::String>(container);
		this->retailPrice= retailPrice;
		this->comment=pdb::makeObject<pdb::String>(comment);
	}

	const pdb::Handle<pdb::String>& getBrand() const {
		return brand;
	}

	void setBrand(const pdb::Handle<pdb::String>& brand) {
		this->brand = brand;
	}

	const pdb::Handle<pdb::String>& getComment() const {
		return comment;
	}

	void setComment(const pdb::Handle<pdb::String>& comment) {
		this->comment = comment;
	}

	const pdb::Handle<pdb::String>& getContainer() const {
		return container;
	}

	void setContainer(const pdb::Handle<pdb::String>& container) {
		this->container = container;
	}

	const pdb::Handle<pdb::String>& getMfgr() const {
		return mfgr;
	}

	void setMfgr(const pdb::Handle<pdb::String>& mfgr) {
		this->mfgr = mfgr;
	}

	const pdb::Handle<pdb::String>& getName() const {
		return name;
	}

	void setName(const pdb::Handle<pdb::String>& name) {
		this->name = name;
	}

	int getPartKey() const {
		return partKey;
	}

	void setPartKey(int partKey) {
		this->partKey = partKey;
	}

	double getRetailPrice() const {
		return retailPrice;
	}

	void setRetailPrice(double retailPrice) {
		this->retailPrice = retailPrice;
	}

	void setSize(int size) {
		this->size = size;
	}

	int getSize() const {
		return size;
	}

	const pdb::Handle<pdb::String>& getType() const {
		return type;
	}

	void setType(const pdb::Handle<pdb::String>& type) {
		this->type = type;
	}
};
#endif