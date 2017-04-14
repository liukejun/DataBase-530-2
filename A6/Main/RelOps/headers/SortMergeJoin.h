
#ifndef SORT_JOIN_H
#define SORT_JOIN_H

#include "MyDB_TableReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_BufferManager.h"
#include <string>
#include <utility>
#include <vector>
#include "Sorting.h"

// This class encapulates a sort merge join.  This is to be used in the case that the
// two input tables are too large to be stored in RAM.
//
class SortMergeJoin {

public:
	// This creates a sort merge join of the tables maanged by leftInput and rightInput.
	//
	// The string finalSelectionPredicate encodes the predicate over record from
	// the table managed by the variable output; only records for which this predicate
	// evaluates to true should be appended to the output table.
	//
	// As records are read in from leftInput (resp. rightInput), they should be 
	// discarded if the predicate encoded by leftSelectionPredicate (resp.
	// rightSelectionPredicate) does not evaluate to true.  
	//
	// Next, the pair equalityCheck encodes a pair of computations, taken from the 
	// predicte finalSelectionPredicate, that must match from the left and the right 
	// records, in order for the final record to be accepted by the predicate.
	// Basically, to run the join, you sort the left relation using equalityCheck.first.
	// You sort the right relation using equalityCheck.second.  Then you merge them
	// using equalityCheck.first and equalityCheck.second.
	//
	// Finally, the vector projections contains all of the computations that are
	// performed to create the output records from the join.
	//
	SortMergeJoin (MyDB_TableReaderWriterPtr leftInput, MyDB_TableReaderWriterPtr rightInput,
		MyDB_TableReaderWriterPtr output, string finalSelectionPredicate, 
		vector <string> projections,
		pair <string, string> equalityCheck, string leftSelectionPredicate,
		string rightSelectionPredicate);
	
	// execute the join
	void run ();
    void mergeRecs (MyDB_RecordPtr leftRec, MyDB_RecordPtr rightRec, vector<MyDB_PageReaderWriter> left, vector<MyDB_PageReaderWriter> right, MyDB_TableReaderWriterPtr output,MyDB_SchemaPtr mySchemaOut);
    int checkBothAcceptance(MyDB_RecordIteratorAltPtr iterL, MyDB_RecordPtr recL, MyDB_RecordIteratorAltPtr iterR, MyDB_RecordPtr recR, func pred);
    int checkSingleAcceptance(func pred, MyDB_RecordIteratorAltPtr iter, MyDB_RecordPtr rec) ;
    int nextState(string equality, vector<MyDB_RecordPtr> vec, MyDB_RecordIteratorAltPtr iter, MyDB_RecordPtr rec, func pred);

private:
    MyDB_TableReaderWriterPtr leftInput;
    MyDB_TableReaderWriterPtr rightInput;
    MyDB_TableReaderWriterPtr output;
    string finalSelectionPredicate;
    vector <string> projections;
    pair <string, string> equalityCheck;
    string leftSelectionPredicate;
    string rightSelectionPredicate;
};

#endif