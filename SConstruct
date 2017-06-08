# SConstuct
# for PDB
# created on: 12/04/2015

import os
import re
import platform
import multiprocessing
import glob
from os import path

common_env = Environment(CXX = 'clang++')
#common_env.Append(YACCFLAGS='-d')
common_env.Append(CFLAGS='-std=c11')

# the following variables are used for output coloring to see errors and warnings better.
common_env = Environment(ENV = {'PATH' : os.environ['PATH'],
                         'TERM' : os.environ['TERM'],
                         'HOME' : os.environ['HOME']})
SRC_ROOT = os.path.join(Dir('.').abspath, "src") # assume the root source dir is in the working dir named as "src"

# OSX settings
if common_env['PLATFORM'] == 'darwin':
    print 'Compiling on OSX'
    common_env.Append(CXXFLAGS = '-std=c++1y -Wall -O0 -g')
    common_env.Replace(CXX = "clang++")

# Linux settings
elif  common_env['PLATFORM'] == 'posix':
    print 'Compiling on Linux'
    common_env.Append(LIBS = ['libdl.so', 'uuid'])

    #for optimization (TODO: O3 not good for Join now)
    #common_env.Append(CXXFLAGS = '-std=c++14 -g -ftree-slp-vectorize -Oz -ldl -fPIC -lstdc++ -Wno-deprecated-declarations')

    #for debugging
    common_env.Append(CXXFLAGS = '-std=c++14 -g  -O0 -ldl -fPIC -lstdc++ -Wno-deprecated-declarations')
    common_env.Append(LINKFLAGS = '-pthread')
    common_env.Replace(CXX = "clang++")

common_env.Append(CCFLAGS='-DINITIALIZE_ALLOCATOR_BLOCK')
common_env.Append(CCFLAGS='-DENABLE_SHALLOW_COPY')
common_env.Append(CCFLAGS='-DDEFAULT_BATCH_SIZE=100')
common_env.Append(CCFLAGS='-DREMOVE_SET_WITH_EVICTION')
common_env.Append(CCFLAGS='-DAUTO_TUNING')
#common_env.Append(CCFLAGS='-DCLEAR_SET')
#common_env.Append(CCFLAGS='-DPDB_DEBUG')
#common_env.Append(CCFLAGS='-DEVICT_STOP_THRESHOLD=0.9')
# Make the build multithreaded
num_cpu = int(multiprocessing.cpu_count())
SetOption('num_jobs', num_cpu)

# two code files that will be included by the VTableMap to pre-load all of the
# built-in object types into the map
objectTargetDir = os.path.join(SRC_ROOT, 'objectModel', 'headers')
def writeIncludeFile(includes):
	#
	# print objectTargetDir + 'BuiltinPDBObjects.h'
	#
	# this is the file where the produced list of includes goes
	includeFile = open(os.path.join(objectTargetDir, 'BuiltinPDBObjects.h'), 'w+')
	includeFile.write ("// Auto-generated by code in SConstruct\n")
	for fileName in includes:
		includeFile.write ('#include "' + fileName + '"\n')

def writeCodeFile(classes):
	#
	# this is the file where the produced code goes
	codeFile = open(os.path.join(objectTargetDir,'BuiltinPDBObjects.cc'), 'w+')
	codeFile.write ("// Auto-generated by code in SConstruct\n\n")
	codeFile.write ("// first, record all of the type codes\n")

	for counter, classname in enumerate(classes, 1):
		codeFile.write ('objectTypeNamesList [getTypeName <' + classname + '> ()] = ' + str(counter) + ';\n')

	codeFile.write ('\n// now, record all of the vTables\n')

	for counter, classname in enumerate(classes, 1):
		codeFile.write ('{\n\tconst UseTemporaryAllocationBlock tempBlock{1024 * 24};');
		codeFile.write ('\n\ttry {\n\t\t')
		codeFile.write (classname + ' tempObject;\n')
		codeFile.write ('\t\tallVTables [' + str(counter) + '] = tempObject.getVTablePtr ();\n')
		codeFile.write ('\t} catch (NotEnoughSpace &e) {\n\t\t')
		codeFile.write ('std :: cout << "Not enough memory to allocate ' + classname + ' to extract the vTable.\\n";\n\t}\n}\n\n');

def writeTypeCodesFile(classes):
	#
	# this is the file where all of the built-in type codes goes
	typeCodesFile = open(os.path.join(objectTargetDir, 'BuiltInObjectTypeIDs.h'), 'w+')
	typeCodesFile.write ("// Auto-generated by code in SConstruct\n")
 	typeCodesFile.write ('#define NoMsg_TYPEID 0\n')

	for counter, classname in enumerate(classes, 1):
		pattern = re.compile('\<[\w\s\<\>]*\>')
		if pattern.search (classname):
			templateArg = pattern.search (classname)
			classname = classname.replace (templateArg.group (), "").strip ()
			#
		# Remove the namespace if any
		classname = classname.rsplit("::")[-1]
		typeCodesFile.write('#define ' + classname + '_TYPEID ' + str(counter) + '\n')

def writeFiles(includes, classes):
	writeIncludeFile(includes)
	writeCodeFile(classes)
	writeTypeCodesFile(classes)

def scanClassNames(includes):
	for counter, fileName in enumerate(includes, 1):
		datafile = file(fileName)
		# search for a line like:
		# // PRELOAD %ObjectTwo%
		p = re.compile('//\s*PRELOAD\s*%[\w\s\<\>]*%')
		for line in datafile:
			# if we found the line
			if p.search(line):
				# extract the text between the two '%' symbols
				m = p.search(line)
				instance = m.group ()
				p = re.compile('%[\w\s\<\>]*%')
				m = p.search(instance)
				yield (m.group ())[1:-1]

def extractCode (common_env, targets, sources, extra_includes=[], extra_classes=[]):
    # Sort the class names so that we have consistent type ids
	scanned = list(scanClassNames(sources))
	scanned.sort()
	writeFiles(
		includes=map(path.abspath, list(extra_includes)) + sources,
		classes=list(extra_classes) + scanned)

# here we get a list of all of the .h files in the 'headers' directory
from os import listdir
from os.path import isfile, isdir, join, abspath
objectheaders = os.path.join(SRC_ROOT, 'builtInPDBObjects', 'headers')
onlyfiles = [abspath(join(objectheaders, f)) for f in listdir(objectheaders) if isfile(join(objectheaders, f)) and f[-2:] == '.h']

# tell scons that the two files 'BuiltinPDBObjects.h' and 'BuiltinPDBObjects.cc' depend on everything in
# the 'headers' directory
common_env.Depends (objectTargetDir + 'BuiltinPDBObjects.h', onlyfiles)
common_env.Depends (objectTargetDir + 'BuiltinPDBObjects.cc', onlyfiles)
common_env.Depends (objectTargetDir + 'BuiltInObjectTypeIDs.h', onlyfiles)

# tell scons that the way to build 'BuiltinPDBObjects.h' and 'BuiltinPDBObjects.cc' is to run extractCode
builtInObjectBuilder = Builder (action = extractCode)
common_env.Append (BUILDERS = {'ExtactCode' : extractCode})
common_env.ExtactCode (
    [ # Target files
        objectTargetDir + 'BuiltinPDBObjects.h',
        objectTargetDir + 'BuiltinPDBObjects.cc',
        objectTargetDir + 'BuiltInObjectTypeIDs.h'
    ],
    onlyfiles, # sources
    )

# Construct a dictionary where each key is the directory basename of a PDB system component folder and each value
# is a list of .cc files used to implement that component.
#
# Expects the path structure of .cc files to be: SRC_ROOT / [component name] / source / [ComponentFile].cc
#
# For example, the structure:
#
#   src/                 <--- assume SRC_ROOT is here
#       compA/
#           headers/
#           source/
#               file1.cc
#               file2.cc
#               readme.txt
#       compB/
#           headers/
#           source/
#               file3.cc
#               file4.cc
#
#
# would result in component_dir_basename_to_cc_file_paths being populated as:
#
#   {'compA':[SRC_ROOT + "/compA/source/file1.cc", SRC_ROOT + "/compA/source/file2.cc"],
#    'compB':[SRC_ROOT + "/compB/source/file3.cc", SRC_ROOT + "/compB/source/file3.cc"]}
#
# on a Linux system.
component_dir_basename_to_lexer_c_file_paths = dict ()
component_dir_basename_to_cc_file_paths = dict ()
component_dir_basename_to_lexer_file_paths = dict ()
src_root_subdir_paths = [path for path in  map(lambda s: join(SRC_ROOT, s), listdir(SRC_ROOT)) if isdir(path)]
for src_subdir_path in src_root_subdir_paths:

    source_folder = join(src_subdir_path, 'source/')
    if(not isdir(source_folder)): # if no source folder lives under the subdir_path, skip this folder
        continue

    src_subdir_basename = os.path.basename(src_subdir_path)

    # first, map build output folders (on the left) to source folders (on the right)
    if src_subdir_basename == 'logicalPlan':
        # maps .y and .l source files used by flex and bison
        lexerSources = [abspath(join(join (source_folder),f2)) for f2 in listdir(source_folder) if isfile(join(source_folder, f2)) and (f2[-2:] == '.y' or f2[-2:] == '.l')]
        component_dir_basename_to_lexer_file_paths [src_subdir_basename] = lexerSources
        
        # maps .cc source files
        common_env.VariantDir(join('build/', src_subdir_basename), [source_folder], duplicate = 0)        
        ccSources = [abspath(join(join ('build/', src_subdir_basename),f2)) for f2 in listdir(source_folder) if isfile(join(source_folder, f2)) and (f2[-3:] == '.cc')]        

        component_dir_basename_to_cc_file_paths [src_subdir_basename] = ccSources

        # maps .c files        
        cSources = [(abspath(join(join ('build/', src_subdir_basename),'Parser.c'))), (abspath(join(join ('build/', src_subdir_basename),'Lexer.c')))]
        
        #component_dir_basename_to_lexer_c_file_paths [src_subdir_basename] = cSources
    
    # Added for LinearAlgebraDSL
    elif src_subdir_basename == 'linearAlgebraDSL':
        # maps .y and .l source files used by flex and bison
        lexerSources = [abspath(join(join (source_folder),f2)) for f2 in listdir(source_folder) if isfile(join(source_folder, f2)) and (f2[-2:] == '.y' or f2[-2:] == '.l')]
        component_dir_basename_to_lexer_file_paths [src_subdir_basename] = lexerSources
        
        # maps .cc source files
        common_env.VariantDir(join('build/', src_subdir_basename), [source_folder], duplicate = 0)        
        ccSources = [abspath(join(join ('build/', src_subdir_basename),f2)) for f2 in listdir(source_folder) if isfile(join(source_folder, f2)) and (f2[-3:] == '.cc')]        

        component_dir_basename_to_cc_file_paths [src_subdir_basename] = ccSources

        # maps .c files        
        cSources = [(abspath(join(join ('build/', src_subdir_basename),'LAParser.c'))), (abspath(join(join ('build/', src_subdir_basename),'LALexer.c')))]
        
        #component_dir_basename_to_lexer_c_file_paths [src_subdir_basename] = cSources

    else:
        common_env.VariantDir(join('build/', src_subdir_basename), [source_folder], duplicate = 0)

        # next, add all of the sources in
        allSources = [abspath(join(join ('build/', src_subdir_basename),f2)) for f2 in listdir(source_folder) if isfile(join(source_folder, f2)) and (f2[-3:] == '.cc' or f2[-2:] == '.y' or f2[-2:] == '.l')]
        component_dir_basename_to_cc_file_paths [src_subdir_basename] = allSources

# second, map build output folders (on the left) to source folders (on the right) for .so libraries
common_env.VariantDir('build/libraries/', 'src/sharedLibraries/source/', duplicate = 0)

# List of folders with headers
headerpaths = [abspath(join(join(SRC_ROOT, f), 'headers/')) for f in listdir(SRC_ROOT) if os.path.isdir (join(join(SRC_ROOT, f), 'headers/'))]

#boost has its own folder structure, which is difficult to be converted to our headers/source structure --Jia
# set BOOST_ROOT and BOOST_SRC_ROOT
BOOST_ROOT = os.path.join(Dir('.').abspath, "src/boost")
BOOST_SRC_ROOT = os.path.join(Dir('.').abspath, "src/boost/libs")
# map all boost source files to a list
boost_component_dir_basename_to_cc_file_paths = dict ()
boost_src_root_subdir_paths = [path for path in  map(lambda s: join(BOOST_SRC_ROOT, s), listdir(BOOST_SRC_ROOT)) if isdir(path)]
for boost_src_subdir_path in boost_src_root_subdir_paths:
        boost_source_folder = join(boost_src_subdir_path, 'src/')
        if(not isdir(boost_source_folder)): # if no source folder lives under the subdir_path, skip this folder
                continue

        boost_src_subdir_basename = os.path.basename(boost_src_subdir_path)

        # first, map build output folders (on the left) to source folders (on the right)
        common_env.VariantDir(join('build/', boost_src_subdir_basename), [boost_source_folder], duplicate = 0)

        # next, add all of the sources in
        allBoostSources = [abspath(join(join ('build/', boost_src_subdir_basename),f2)) for f2 in listdir(boost_source_folder) if isfile(join(boost_source_folder, f2)) and f2[-4:] == '.cpp']
        boost_component_dir_basename_to_cc_file_paths [boost_src_subdir_basename] = allBoostSources



# append boost to headerpaths
headerpaths.append(BOOST_ROOT)



# Adds header folders and required libraries
common_env.Append(CPPPATH = headerpaths)

print 'Platform: ' + platform.platform()
print 'System: ' + platform.system()
print 'Release: ' + platform.release()
print 'Version: ' + platform.version()

all = ['build/sqlite/sqlite3.c',
       component_dir_basename_to_cc_file_paths['serverFunctionalities'],
#       component_dir_basename_to_cc_file_paths['bufferMgr'],
       component_dir_basename_to_cc_file_paths['communication'],
       component_dir_basename_to_cc_file_paths['catalog'],
       component_dir_basename_to_cc_file_paths['dispatcher'],
       component_dir_basename_to_cc_file_paths['pdbServer'],
       component_dir_basename_to_cc_file_paths['objectModel'],
       component_dir_basename_to_cc_file_paths['queryExecution'],
       component_dir_basename_to_cc_file_paths['queryPlanning'],
       component_dir_basename_to_cc_file_paths['queryIntermediaryRep'], 
       component_dir_basename_to_cc_file_paths['work'],
       component_dir_basename_to_cc_file_paths['memory'],
       component_dir_basename_to_cc_file_paths['storage'],
       component_dir_basename_to_cc_file_paths['distributionManager'],
#       component_dir_basename_to_cc_file_paths['tcapLexer'],
#       component_dir_basename_to_cc_file_paths['tcapParser'],
#       component_dir_basename_to_cc_file_paths['tcapIntermediaryRep'],
       component_dir_basename_to_cc_file_paths['logicalPlan'],
       component_dir_basename_to_cc_file_paths['linearAlgebraDSL'],
       component_dir_basename_to_cc_file_paths['lambdas'],
       component_dir_basename_to_lexer_file_paths['logicalPlan'],
       component_dir_basename_to_lexer_file_paths['linearAlgebraDSL'],
       boost_component_dir_basename_to_cc_file_paths['filesystem'],
       boost_component_dir_basename_to_cc_file_paths['program_options'],
#       boost_component_dir_basename_to_cc_file_paths['smart_ptr'],
       boost_component_dir_basename_to_cc_file_paths['system'],
      ]

common_env.SharedLibrary('libraries/libAllSelection.so', ['build/libraries/AllSelection.cc'] + all)
common_env.SharedLibrary('libraries/libAllSelectionWithCreation.so', ['build/libraries/AllSelectionWithCreation.cc'] + all)
common_env.SharedLibrary('libraries/libCartesianJoin.so', ['build/libraries/CartesianJoin.cc'] + all)
common_env.SharedLibrary('libraries/libChrisSelection.so', ['build/libraries/ChrisSelection.cc'] + all)
common_env.SharedLibrary('libraries/libEmployeeBuiltInIdentitySelection.so', ['build/libraries/EmployeeBuiltInIdentitySelection.cc'] + all)
common_env.SharedLibrary('libraries/libEmployeeIdentitySelection.so', ['build/libraries/EmployeeIdentitySelection.cc'] + all)
common_env.SharedLibrary('libraries/libEmployeeSelection.so', ['build/libraries/EmployeeSelection.cc'] + all)
common_env.SharedLibrary('libraries/libFinalSelection.so', ['build/libraries/FinalSelection.cc'] + all)
common_env.SharedLibrary('libraries/libIntAggregation.so', ['build/libraries/IntAggregation.cc'] + all)
common_env.SharedLibrary('libraries/libIntSelectionOfStringIntPair.so', ['build/libraries/IntSelectionOfStringIntPair.cc'] + all)
common_env.SharedLibrary('libraries/libIntSillyJoin.so', ['build/libraries/IntSillyJoin.cc'] + all)
common_env.SharedLibrary('libraries/libKMeansQuery.so', ['build/libraries/KMeansQuery.cc'] + all)

common_env.SharedLibrary('libraries/libLAMaxElementOutputType.so', ['build/libraries/LAMaxElementOutputType.cc'] + all)
common_env.SharedLibrary('libraries/libLAMaxElementValueType.so', ['build/libraries/LAMaxElementValueType.cc'] + all)
common_env.SharedLibrary('libraries/libLAMinElementOutputType.so', ['build/libraries/LAMinElementOutputType.cc'] + all)
common_env.SharedLibrary('libraries/libLAMinElementValueType.so', ['build/libraries/LAMinElementValueType.cc'] + all)
common_env.SharedLibrary('libraries/libLAScanMatrixBlockSet.so', ['build/libraries/LAScanMatrixBlockSet.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyAddJoin.so', ['build/libraries/LASillyAddJoin.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyRowMaxAggregate.so', ['build/libraries/LASillyRowMaxAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyRowMinAggregate.so', ['build/libraries/LASillyRowMinAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyColMaxAggregate.so', ['build/libraries/LASillyColMaxAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyColMinAggregate.so', ['build/libraries/LASillyColMinAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillySubstractJoin.so', ['build/libraries/LASillySubstractJoin.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyMaxElementAggregate.so', ['build/libraries/LASillyMaxElementAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyMinElementAggregate.so', ['build/libraries/LASillyMinElementAggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyMultiply1Join.so', ['build/libraries/LASillyMultiply1Join.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyMultiply2Aggregate.so', ['build/libraries/LASillyMultiply2Aggregate.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyTransposeMultiply1Join.so', ['build/libraries/LASillyTransposeMultiply1Join.cc'] + all)
common_env.SharedLibrary('libraries/libLASillyTransposeSelection.so', ['build/libraries/LASillyTransposeSelection.cc'] + all)
common_env.SharedLibrary('libraries/libLAWriteMatrixBlockSet.so', ['build/libraries/LAWriteMatrixBlockSet.cc'] + all)
common_env.SharedLibrary('libraries/libLAWriteMaxElementSet.so', ['build/libraries/LAWriteMaxElementSet.cc'] + all)
common_env.SharedLibrary('libraries/libLAWriteMinElementSet.so', ['build/libraries/LAWriteMinElementSet.cc'] + all)
common_env.SharedLibrary('libraries/libMatrixMeta.so', ['build/libraries/MatrixMeta.cc'] + all)
common_env.SharedLibrary('libraries/libMatrixData.so', ['build/libraries/MatrixData.cc'] + all)
common_env.SharedLibrary('libraries/libMatrixBlock.so', ['build/libraries/MatrixBlock.cc'] + all)

common_env.SharedLibrary('libraries/libPartialResult.so', ['build/libraries/PartialResult.cc'] + all)
common_env.SharedLibrary('libraries/libScanBuiltinEmployeeSet.so', ['build/libraries/ScanBuiltinEmployeeSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanEmployeeSet.so', ['build/libraries/ScanEmployeeSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanIntSet.so', ['build/libraries/ScanIntSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanSimpleEmployeeSet.so', ['build/libraries/ScanSimpleEmployeeSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanStringIntPairSet.so', ['build/libraries/ScanStringIntPairSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanStringSet.so', ['build/libraries/ScanStringSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanSupervisorSet.so', ['build/libraries/ScanSupervisorSet.cc'] + all)
common_env.SharedLibrary('libraries/libSharedEmployee.so', ['build/libraries/SharedEmployee.cc'] + all)
common_env.SharedLibrary('libraries/libSharedEmployeeTopK.so', ['build/libraries/SharedEmployeeTopK.cc'] + all)
common_env.SharedLibrary('libraries/libSillyAggregation.so', ['build/libraries/SillyAggregation.cc'] + all)
common_env.SharedLibrary('libraries/libSillyJoin.so', ['build/libraries/SillyJoin.cc'] + all)
common_env.SharedLibrary('libraries/libSillySelection.so', ['build/libraries/SillySelection.cc'] + all)
common_env.SharedLibrary('libraries/libSimpleAggregation.so', ['build/libraries/SimpleAggregation.cc'] + all)
common_env.SharedLibrary('libraries/libSimpleEmployee.so', ['build/libraries/SimpleEmployee.cc'] + all)
common_env.SharedLibrary('libraries/libStringSelection.so', ['build/libraries/StringSelection.cc'] + all)
common_env.SharedLibrary('libraries/libStringSelectionOfStringIntPair.so', ['build/libraries/StringSelectionOfStringIntPair.cc'] + all)
common_env.SharedLibrary('libraries/libWriteBuiltinEmployeeSet.so', ['build/libraries/WriteBuiltinEmployeeSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteDoubleSet.so', ['build/libraries/WriteDoubleSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteEmployeeSet.so', ['build/libraries/WriteEmployeeSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteIntSet.so', ['build/libraries/WriteIntSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteStringIntPairSet.so', ['build/libraries/WriteStringIntPairSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteStringSet.so', ['build/libraries/WriteStringSet.cc'] + all)
common_env.SharedLibrary('libraries/libWriteSumResultSet.so', ['build/libraries/WriteSumResultSet.cc'] + all)
common_env.SharedLibrary('libraries/libDoubleVectorAggregation.so', ['build/libraries/DoubleVectorAggregation.cc'] + all)
common_env.SharedLibrary('libraries/libSupervisorMultiSelection.so', ['build/libraries/SupervisorMultiSelection.cc'] + all)
common_env.SharedLibrary('libraries/libSillyGroupBy.so', ['build/libraries/SillyGroupBy.cc'] + all)
common_env.SharedLibrary('libraries/libEmployeeGroupBy.so', ['build/libraries/EmployeeGroupBy.cc'] + all)

# TPCH Benchmakr Libraries
common_env.SharedLibrary('libraries/libPart.so', ['build/tpchBench/Part.cc'] + all)
common_env.SharedLibrary('libraries/libSupplier.so', ['build/tpchBench/Supplier.cc'] + all)
common_env.SharedLibrary('libraries/libLineItem.so', ['build/tpchBench/LineItem.cc'] + all)
common_env.SharedLibrary('libraries/libOrder.so', ['build/tpchBench/Order.cc'] + all)
common_env.SharedLibrary('libraries/libCustomer.so', ['build/tpchBench/Customer.cc'] + all)
common_env.SharedLibrary('libraries/libCustomerMultiSelection.so', ['build/tpchBench/CustomerMultiSelection.cc'] + all)
common_env.SharedLibrary('libraries/libOrderMultiSelection.so', ['build/tpchBench/OrderMultiSelection.cc'] + all)
common_env.SharedLibrary('libraries/libCustomerSupplierPartWriteSet.so', ['build/tpchBench/CustomerSupplierPartWriteSet.cc'] + all)
common_env.SharedLibrary('libraries/libScanCustomerSet.so', ['build/tpchBench/ScanCustomerSet.cc'] + all)
common_env.SharedLibrary('libraries/libCustomerSupplierPart.so', ['build/tpchBench/CustomerSupplierPart.cc'] + all)


common_env.Program('bin/tpchDataGenerator', ['build/tpchBench/tpchDataGenerator.cc'] + all)






common_env.Program('bin/CatalogServerTests', ['build/tests/CatalogServerTests.cc'] + all)
common_env.Program('bin/CatalogTests', ['build/tests/CatalogTests.cc'] + all)
common_env.Program('bin/getListNodesTest', ['build/tests/GetListNodesTest.cc'] + all)
common_env.Program('bin/MasterServerTest', ['build/tests/MasterServerTest.cc'] + all)
common_env.Program('bin/objectModelTest1', ['build/tests/ObjectModelTest1.cc'] + all)
common_env.Program('bin/pdb-cluster', ['build/tests/Test404.cc'] + all)
common_env.Program('bin/pdb-server', ['build/tests/Test603.cc']+all)
common_env.Program('bin/pdbServer', ['build/mainServer/PDBMainServerInstance.cc'] + all)
common_env.Program('bin/registerTypeAndCreateDatabaseTest', ['build/tests/RegisterTypeAndCreateDatabaseTest.cc'] + all)
common_env.Program('bin/storeLotsOfEmployee', ['build/tests/StoreLotsOfEmployee.cc'] + all)
common_env.Program('bin/storeSharedEmployeeInDBTest', ['build/tests/StoreSharedEmployeeInDBTest.cc'] + all)
common_env.Program('bin/test1', ['build/tests/Test1.cc'] + all)
common_env.Program('bin/test2', ['build/tests/Test2.cc'] + all)
common_env.Program('bin/test3', ['build/tests/Test3.cc'] + all)
common_env.Program('bin/test4', ['build/tests/Test4.cc'] + all)
common_env.Program('bin/test5', ['build/tests/Test5.cc'] + all)
common_env.Program('bin/test6', ['build/tests/Test6.cc'] + all)
common_env.Program('bin/test7', ['build/tests/Test7.cc'] + all)
common_env.Program('bin/test8', ['build/tests/Test8.cc'] + all)
common_env.Program('bin/test9', ['build/tests/Test9.cc'] + all)
common_env.Program('bin/test10', ['build/tests/Test10.cc'] + all)
common_env.Program('bin/test100', ['build/tests/Test100.cc'] + all)
common_env.Program('bin/test11', ['build/tests/Test11.cc'] + all)
common_env.Program('bin/test12', ['build/tests/Test12.cc'] + all)
#common_env.Program('bin/test13', ['build/tests/Test13.cc'] + all)
common_env.Program('bin/test14', ['build/tests/Test14.cc'] + all)
common_env.Program('bin/test16', ['build/tests/Test16.cc'] + all)
common_env.Program('bin/test18', ['build/tests/Test18.cc'] + all)
common_env.Program('bin/test19', ['build/tests/Test19.cc'] + all)
common_env.Program('bin/test20', ['build/tests/Test20.cc'] + all)
common_env.Program('bin/test21', ['build/tests/Test21.cc'] + all)
common_env.Program('bin/test22', ['build/tests/Test22.cc'] + all)
common_env.Program('bin/test23', ['build/tests/Test23.cc'] + all)
common_env.Program('bin/test24', ['build/tests/Test24.cc'] + all)
common_env.Program('bin/test24-temp', ['build/tests/Test24-temp.cc'] + all)
common_env.Program('bin/test25', ['build/tests/Test25.cc'] + all)
common_env.Program('bin/test26', ['build/tests/Test26.cc'] + all)
common_env.Program('bin/test27', ['build/tests/Test27.cc'] + all)
common_env.Program('bin/test28', ['build/tests/Test28.cc'] + all)
common_env.Program('bin/test29', ['build/tests/Test29.cc'] + all)
common_env.Program('bin/test30', ['build/tests/Test30.cc'] + all)
common_env.Program('bin/test31', ['build/tests/Test31.cc'] + all)
common_env.Program('bin/test32', ['build/tests/Test32.cc'] + all)
common_env.Program('bin/test33', ['build/tests/Test33.cc'] + all)
common_env.Program('bin/test34', ['build/tests/Test34.cc'] + all)
common_env.Program('bin/test35', ['build/tests/Test35.cc'] + all)
common_env.Program('bin/test36', ['build/tests/Test36.cc'] + all)
common_env.Program('bin/test37', ['build/tests/Test37.cc'] + all)
common_env.Program('bin/test38', ['build/tests/Test38.cc'] + all)
common_env.Program('bin/test39', ['build/tests/Test39.cc'] + all)
common_env.Program('bin/test4', ['build/tests/Test4.cc'] + all)
common_env.Program('bin/test40', ['build/tests/Test40.cc'] + all)
common_env.Program('bin/test400', ['build/tests/Test400.cc'] + all)
common_env.Program('bin/test401', ['build/tests/Test401.cc'] + all)
common_env.Program('bin/test402', ['build/tests/Test402.cc'] + all)
common_env.Program('bin/test403', ['build/tests/Test403.cc'] + all)
common_env.Program('bin/test405', ['build/tests/Test405.cc'] + all)
common_env.Program('bin/test42', ['build/tests/Test42.cc'] + all)
common_env.Program('bin/test43', ['build/tests/Test43.cc'] + all)
common_env.Program('bin/test44', ['build/tests/Test44.cc'] + all)
common_env.Program('bin/test45', ['build/tests/Test45.cc'] + all)
common_env.Program('bin/test46', ['build/tests/Test46.cc'] + all)
common_env.Program('bin/test47', ['build/tests/Test47.cc'] + all)
common_env.Program('bin/test47Join', ['build/tests/Test47Join.cc'] + all)
common_env.Program('bin/test47JoinB', ['build/tests/Test47JoinB.cc'] + all)
common_env.Program('bin/test47JoinC', ['build/tests/Test47JoinC.cc'] + all)
common_env.Program('bin/test47JoinD', ['build/tests/Test47JoinD.cc'] + all)
common_env.Program('bin/test48', ['build/tests/Test48.cc'] + all)
common_env.Program('bin/test49', ['build/tests/Test49.cc'] + all)
common_env.Program('bin/test50', ['build/tests/Test50.cc'] + all)
common_env.Program('bin/test51', ['build/tests/Test51.cc'] + all)
common_env.Program('bin/test52', ['build/tests/Test52.cc'] + all)
common_env.Program('bin/test53', ['build/tests/Test53.cc'] + all)
common_env.Program('bin/test54', ['build/tests/Test54.cc'] + all)
common_env.Program('bin/test56', ['build/tests/Test56.cc'] + all)
common_env.Program('bin/test57', ['build/tests/Test57.cc'] + all)
common_env.Program('bin/test58', ['build/tests/Test58.cc'] + all)
common_env.Program('bin/test60', ['build/tests/Test60.cc'] + all)
common_env.Program('bin/test62', ['build/tests/Test62.cc'] + all)
common_env.Program('bin/test63', ['build/tests/Test63.cc'] + all)
common_env.Program('bin/test64', ['build/tests/Test64.cc'] + all)
common_env.Program('bin/test65', ['build/tests/Test65.cc'] + all)
common_env.Program('bin/test66', ['build/tests/Test66.cc'] + all)
common_env.Program('bin/test67', ['build/tests/Test67.cc'] + all)
common_env.Program('bin/test68', ['build/tests/Test68.cc'] + all)
common_env.Program('bin/test69', ['build/tests/Test69.cc'] + all)
common_env.Program('bin/test70', ['build/tests/Test70.cc'] + all)
common_env.Program('bin/test71', ['build/tests/Test71.cc'] + all)
common_env.Program('bin/test72', ['build/tests/Test72.cc'] + all)
common_env.Program('bin/test73', ['build/tests/Test73.cc'] + all)
common_env.Program('bin/test74', ['build/tests/Test74.cc'] + all)
common_env.Program('bin/test75', ['build/tests/Test75.cc'] + all)
common_env.Program('bin/test76', ['build/tests/Test76.cc'] + all)
common_env.Program('bin/test77', ['build/tests/Test77.cc'] + all)
common_env.Program('bin/test78', ['build/tests/Test78.cc'] + all)
common_env.Program('bin/test79', ['build/tests/Test79.cc'] + all)
common_env.Program('bin/test80', ['build/tests/Test80.cc'] + all)
common_env.Program('bin/test81builtIn', ['build/tests/Test81builtIn.cc'] + all)
common_env.Program('bin/test81shared', ['build/tests/Test81shared.cc'] + all)
#common_env.Program('bin/test82', ['build/tests/Test82.cc'] + all)
common_env.Program('bin/test83', ['build/tests/Test83.cc'] + all)
common_env.Program('bin/test84', ['build/tests/Test84.cc'] + all)
common_env.Program('bin/test85', ['build/tests/Test85.cc'] + all)
common_env.Program('bin/test86', ['build/tests/Test86.cc'] + all)
common_env.Program('bin/test87', ['build/tests/Test87.cc'] + all)
common_env.Program('bin/test88', ['build/tests/Test88.cc'] + all)
common_env.Program('bin/testLA01_Transpose', ['build/tests/TestLA01_Transpose.cc'] + all)
common_env.Program('bin/testLA02_Add', ['build/tests/TestLA02_Add.cc'] + all)
common_env.Program('bin/testLA03_Substract', ['build/tests/TestLA03_Substract.cc'] + all)
common_env.Program('bin/testLA04_Multiply', ['build/tests/TestLA04_Multiply.cc'] + all)
common_env.Program('bin/testLA05_MaxElement', ['build/tests/TestLA05_MaxElement.cc'] + all)
common_env.Program('bin/testLA06_MinElement', ['build/tests/TestLA06_MinElement.cc'] + all)
common_env.Program('bin/testLA07_TransposeMultiply', ['build/tests/TestLA07_TransposeMultiply.cc'] + all)
common_env.Program('bin/testLA08_RowMax', ['build/tests/TestLA08_RowMax.cc'] + all)
common_env.Program('bin/testLA09_RowMin', ['build/tests/TestLA09_RowMin.cc'] + all)
common_env.Program('bin/testLA10_ColMax', ['build/tests/TestLA10_ColMax.cc'] + all)
common_env.Program('bin/testLA11_ColMin', ['build/tests/TestLA11_ColMin.cc'] + all)
common_env.Program('bin/testLA20_Parser', ['build/tests/TestLA20_Parser.cc'] + all)

common_env.Program('bin/tpchTestData', ['build/tpchBench/TestTPCHData.cc'] + all)




#Testing
pdbTest=common_env.Command('test', 'scripts/integratedTests.py', 'python $SOURCE -o $TARGET')
#pdbTest=common_env.Command('test',['bin/test603', 'bin/test46', 'bin/test44','libraries/libStringSelection.so', 'libraries/libChrisSelection.so', 'libraries/libSharedEmployee.so'],'python scripts/integratedTests.py -o $TARGET')
common_env.Depends(pdbTest, [
  'bin/CatalogTests',
  'bin/pdb-cluster',
  'bin/pdb-server', 
  'bin/test44', 
  'bin/test46', 
  'bin/test52', 
  'bin/test74', 
  'bin/test79', 
  'bin/test78', 
  'libraries/libCartesianJoin.so', 
  'libraries/libChrisSelection.so', 
  'libraries/libEmployeeSelection.so',
  'libraries/libFinalSelection.so', 
  'libraries/libIntAggregation.so', 
  'libraries/libIntSelectionOfStringIntPair.so', 
  'libraries/libIntSillyJoin.so', 
  'libraries/libScanEmployeeSet.so',
  'libraries/libScanIntSet.so', 
  'libraries/libScanSimpleEmployeeSet.so', 
  'libraries/libScanStringIntPairSet.so', 
  'libraries/libScanStringSet.so', 
  'libraries/libScanSupervisorSet.so', 
  'libraries/libSharedEmployee.so', 
  'libraries/libSillyAggregation.so', 
  'libraries/libSillyJoin.so', 
  'libraries/libSillySelection.so', 
  'libraries/libSimpleEmployee.so', 
  'libraries/libStringSelection.so', 
  'libraries/libStringSelectionOfStringIntPair.so', 
  'libraries/libWriteIntSet.so', 
  'libraries/libWriteStringIntPairSet.so', 
  'libraries/libWriteStringSet.so',
  'libraries/libWriteSumResultSet.so'
])
common_env.Alias('tests', pdbTest)
main=common_env.Alias('main', [
  'bin/CatalogServerTests',
  'bin/CatalogTests', 
#  'bin/getListNodesTest', 
#  'bin/MasterServerTest', 
#  'bin/objectModelTest1', 
  'bin/pdb-cluster', 
  'bin/pdb-server', 
#  'bin/pdbServer', 
#  'bin/registerTypeAndCreateDatabaseTest', 
#  'bin/storeLotsOfEmployee',
#  'bin/storeSharedEmployeeInDBTest', 
#  'bin/test1', 
#  'bin/test2', 
#  'bin/test3', 
#  'bin/test4', 
#  'bin/test5', 
#  'bin/test6', 
#  'bin/test7', 
#  'bin/test8', 
#  'bin/test9', 
#  'bin/test10', 
#  'bin/test11', 
#  'bin/test12', 
#  'bin/test16', 
#  'bin/test44', 
#  'bin/test46', 
  'bin/test47', 
  'bin/test47Join', 
  'bin/test47JoinB', 
  'bin/test47JoinC',
  'bin/test47JoinD',
#  'bin/test52', 
#  'bin/test53', 
#  'bin/test54', 
#  'bin/test56', 
#  'bin/test57', 
#  'bin/test58', 
#  'bin/test60', 
#  'bin/test62', 
#  'bin/test63', 
#  'bin/test64', 
#  'bin/test65', 
  'bin/test66', 
  'bin/test67', 
  'bin/test68', 
  'bin/test69', 
  'bin/test70', 
  'bin/test71', 
  'bin/test72', 
  'bin/test73', 
  'bin/test74', 
  'bin/test75', 
  'bin/test76', 
  'bin/test77', 
  'bin/test78', 
  'bin/test79', 
  'bin/test80',
  'bin/test81builtIn',
  'bin/test81shared',
  #'bin/test82',
  'bin/test83',
  'bin/test84',
  'bin/test85',
  'bin/test86',
  'bin/test87',
  'bin/test88',
  #'bin/testLA01_Transpose',
  #'bin/testLA02_Add',
  #'bin/testLA03_Substract',
  #'bin/testLA04_Multiply',
  #'bin/testLA05_MaxElement',
  #'bin/testLA06_MinElement',
  #'bin/testLA07_TransposeMultiply',
  #'bin/testLA08_RowMax',
  #'bin/testLA09_RowMin',
  #'bin/testLA10_ColMax',
  #'bin/testLA11_ColMin',
  'bin/testLA20_Parser',

  'libraries/libAllSelection.so', 
  'libraries/libAllSelectionWithCreation.so', 
  'libraries/libCartesianJoin.so', 
  'libraries/libChrisSelection.so',
  'libraries/libDoubleVectorAggregation.so', 
  'libraries/libEmployeeSelection.so',
  'libraries/libEmployeeBuiltInIdentitySelection.so',
  'libraries/libEmployeeIdentitySelection.so',
  'libraries/libFinalSelection.so', 
  'libraries/libIntAggregation.so', 
  'libraries/libIntSelectionOfStringIntPair.so', 
  'libraries/libIntSillyJoin.so', 
  'libraries/libSillyGroupBy.so',
  'libraries/libEmployeeGroupBy.so',
#  'libraries/libKMeansQuery.so',  
  
  # TPCH Benchamr Libraries
  'libraries/libPart.so',
  'libraries/libSupplier.so',
  'libraries/libLineItem.so',
  'libraries/libOrder.so',
  'libraries/libCustomer.so',
  'libraries/libCustomerMultiSelection.so',
  'libraries/libOrderMultiSelection.so',
  'libraries/libCustomerSupplierPartWriteSet.so',
  'libraries/libScanCustomerSet.so',
  'libraries/libCustomerSupplierPart.so',
# 'bin/tpchTestData',
  'bin/tpchDataGenerator',
  
  
  
  #'libraries/libLAMaxElementValueType.so',
  #'libraries/libLAMaxElementOutputType.so',
  #'libraries/libLAMinElementValueType.so',
  #'libraries/libLAMinElementOutputType.so',
  #'libraries/libLAScanMatrixBlockSet.so',
  #'libraries/libLASillyAddJoin.so',
  #'libraries/libLASillyRowMaxAggregate.so',
  #'libraries/libLASillyRowMinAggregate.so',
  #'libraries/libLASillyColMaxAggregate.so',
  #'libraries/libLASillyColMinAggregate.so',
  #'libraries/libLASillyMaxElementAggregate.so',
  #'libraries/libLASillyMinElementAggregate.so',
  #'libraries/libLASillyMultiply1Join.so',
  #'libraries/libLASillyMultiply2Aggregate.so',
  #'libraries/libLASillySubstractJoin.so',
  #'libraries/libLASillyTransposeSelection.so',
  #'libraries/libLASillyTransposeMultiply1Join.so',
  #'libraries/libLAWriteMatrixBlockSet.so',
  #'libraries/libLAWriteMaxElementSet.so',
  #'libraries/libLAWriteMinElementSet.so',
  #'libraries/libMatrixMeta.so',
  #'libraries/libMatrixData.so',
  #'libraries/libMatrixBlock.so',
  
 # 'libraries/libPartialResult.so', 
  'libraries/libScanBuiltinEmployeeSet.so', 
  'libraries/libScanEmployeeSet.so',
  'libraries/libScanIntSet.so', 
  'libraries/libScanSimpleEmployeeSet.so', 
  'libraries/libScanStringIntPairSet.so', 
  'libraries/libScanStringSet.so', 
  'libraries/libScanSupervisorSet.so', 
  'libraries/libSharedEmployee.so', 
  'libraries/libSharedEmployeeTopK.so', 
  'libraries/libSillyAggregation.so', 
  'libraries/libSillyJoin.so', 
  'libraries/libSillySelection.so', 
  'libraries/libSimpleAggregation.so', 
  'libraries/libSimpleEmployee.so', 
  'libraries/libStringSelection.so', 
  'libraries/libStringSelectionOfStringIntPair.so', 
  'libraries/libSupervisorMultiSelection.so',
  'libraries/libWriteBuiltinEmployeeSet.so', 
  'libraries/libWriteDoubleSet.so', 
  'libraries/libWriteEmployeeSet.so', 
  'libraries/libWriteIntSet.so', 
  'libraries/libWriteStringIntPairSet.so', 
  'libraries/libWriteStringSet.so',
  'libraries/libWriteSumResultSet.so'
])
Default(main)
