
#include "Lexer.h"
#include "Parser.h"
#include "ParserTypes.h"
#include "MyDB_BufferManager.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include <string>      
#include <iostream>   
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdio.h>
using namespace std;
string toLower (string data) {
	transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}


//int main ()
//{
//    if( remove( "/Users/kejunliu/Documents/DataBase-530-2/A7_1/Build/bin/Join49Out.bin" ) != 0 )
//        perror( "Error deleting file" );
//    else
//        puts( "File successfully deleted" );
//    return 0;
//}
int main (int numArgs, char **args) {
    
//	// make sure we have the correct arguments
//	if (numArgs != 3) {
//		cout << "args: catalog_file directory_for_tables\n";
//		return 0;
//	}

	// open up the catalog file
	//MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> (args [1]);
    MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");

	// start up the buffer manager

	MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (131072, 9056, "tempFile");


	// and create tables for everything in the database
	static map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);

	// this is all of the tables
	static map <string, MyDB_TableReaderWriterPtr> allTableReaderWriters;

	// and this is just the B+-Trees
	static map <string, MyDB_BPlusTreeReaderWriterPtr> allBPlusReaderWriters;

	// load 'em up
	for (auto &a : allTables) {
		if (a.second->getFileType () == "heap") {
			allTableReaderWriters[a.first] =  make_shared <MyDB_TableReaderWriter> (a.second, myMgr);
		} else if (a.second->getFileType () == "bplustree") {
			allBPlusReaderWriters[a.first] = make_shared <MyDB_BPlusTreeReaderWriter> (a.second->getSortAtt (), a.second, myMgr);
			allTableReaderWriters[a.first] = allBPlusReaderWriters[a.first];	
		}
	}

	// print out the intro notification
	cout << "\n          Welcome to MyDB v0.1\n\n";
	cout << "\"Not the worst database in the world\" (tm) \n\n";

	// and repeatedly accept queries
	while (true) {
		
		cout << "MyDB> ";

		// this will be used to collect the query
		stringstream ss;

		// get a line
		for (string line; getline (cin, line);) {
			
			// see if it has a ";" at the end
			size_t pos = line.find (';');

			// it does!!  so we are ready yo parse
			if (pos != string :: npos) {

				// append the last line
				line.resize (pos);
				ss << line;

				// see if someone wants to load a file
				vector <string> tokens {istream_iterator<string>{ss},
					istream_iterator<string>{}};

				// see if we got a "quit" or "exit"
				if (tokens.size () == 1 && (toLower (tokens[0]) == "exit" || toLower (tokens[0]) == "quit")) {
					cout << "OK, goodbye.\n";
					// before we get outta here, write everything into the catalog
					for (auto &a : allTables) {
						a.second->putInCatalog (myCatalog);
					}
					return 0;
				}

				// see if we got a "load soandso from afile"
				if (tokens.size () == 4 && toLower(tokens[0]) == "load" && toLower(tokens[2]) == "from") {

					// make sure the table is there
					if (allTableReaderWriters.count (tokens[1]) == 0) {
						cout << "Could not find table " << tokens[1] << ".\n";
						break;
					} else {
                        cout << "OK, loading " << tokens[1] << " from text file.\n";
                        
                        // load up the file
                        pair <vector <size_t>, size_t> res = allTableReaderWriters[tokens[1]]->loadFromTextFile (tokens[3]);
                        
                        // and record the tuple various counts
                        allTableReaderWriters[tokens[1]]->getTable ()->setDistinctValues (res.first);
                        allTableReaderWriters[tokens[1]]->getTable ()->setTupleCount (res.second);
                        break;
                    }
                    
				}

				// get the string to parse
				string parseMe = ss.str ();

				// see if we got a load-from
				
				// add an extra zero at the end; needed by lexer
				parseMe.push_back ('\0');

				// now parse it
				yyscan_t scanner;
				LexerExtra extra { "" };
				yylex_init_extra (&extra, &scanner);
				const YY_BUFFER_STATE buffer { yy_scan_string (parseMe.data(), scanner) };
				SQLStatement *final = nullptr;
				const int parseFailed { yyparse (scanner, &final) };
				yy_delete_buffer (buffer, scanner);
				yylex_destroy (scanner);

				// if we did not parse correctly
				if (parseFailed) {

					// print out the parse error
					cout << "Parse error: " << extra.errorMessage;

					// get outta here
					if (final != nullptr)
						delete final;
					break;

				// if we did parse correctly, just print out the query
				} else {

					// see if we got a create table
					if (final->isCreateTable ()) {

						string tableName = final->addToCatalog ("/Users/xiajunru/Code/DataBase-530-2/A7_1/Build/bin", myCatalog);
						if (tableName != "nothing") {
							allTables = MyDB_Table :: getAllTables (myCatalog);
							if (allTables [tableName]->getFileType () == "heap") {
								allTableReaderWriters[tableName] = 
									make_shared <MyDB_TableReaderWriter> (allTables [tableName], myMgr);
							} else if (allTables [tableName]->getFileType () == "bplustree") {
								allBPlusReaderWriters[tableName] = 
									make_shared <MyDB_BPlusTreeReaderWriter> 
										(allTables [tableName]->getSortAtt (), allTables [tableName], myMgr);
								allTableReaderWriters[tableName] = allBPlusReaderWriters[tableName];
							}
//							cout << "Added table " << final->addToCatalog ("/Users/xiajunru/Code/DataBase-530-2/A7_1/Build/bin", myCatalog) << "\n";
//                            
                            map <string, MyDB_TablePtr> allTables = MyDB_Table::getAllTables(myCatalog);
//                            for (std::map<string, MyDB_TablePtr>::iterator it=allTables.begin(); it!=allTables.end(); ++it)
//                                std::cout << it->first << " => " << *it->second << "\n";
//                            cout << "create TableReaderWriter..." << flush;
                            
                            
                        }	

					} else if (final->isSFWQuery ()) {

						// print it out
						final->printSFWQuery (myCatalog, myMgr, allTableReaderWriters, "/Users/xiajunru/Code/DataBase-530-2/A7_1/Build/bin");
					}

					// get outta here
					if (final != nullptr)
						delete final;
					break;
				}

			} else {
				ss << line << "\n";
				cout << "    > ";
			}
		}
	}

}
