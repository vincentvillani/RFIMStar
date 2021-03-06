current_dir = $(shell pwd)


CC_FLAGS=-DMKL_ILP64 -m64 -DBUILD_WITH_MKL
LIBS=-lmkl_intel_ilp64 -lmkl_core -lmkl_sequential -lpthread -lm -ldl
INCLUDES=-isystem $(current_dir)
DEBUG_FLAGS=-g3 -O0 -std=c++11
RELEASE_FLAGS=-O3 -std=c++11
FILE_LIST=Main.cpp Source/MasterMailbox.cpp Source/ReaderThread.cpp Source/ReaderThreadData.cpp Source/ReaderWorkerMailbox.cpp Source/RFIMConfiguration.cpp Source/RFIMHelperFunctions.cpp Source/RFIMMemoryBlock.cpp Source/RFIMStar.cpp Source/UnitTests.cpp Source/WorkerThread.cpp Source/WorkerThreadData.cpp Source/WorkerWriterMailbox.cpp Source/WriterReaderMailbox.cpp Source/WriterThread.cpp Source/WriterThreadData.cpp

debug:
	g++ $(CC_FLAGS) $(INCLUDES) -o RFIMStarDebug $(FILE_LIST) $(LIBS) $(DEBUG_FLAGS)


release:
	g++ $(CC_FLAGS) $(INCLUDES) -o RFIMStarRelease $(FILE_LIST) $(LIBS) $(RELEASE_FLAGS)


clean:
	rm RFIMStarDebug RFIMStarRelease