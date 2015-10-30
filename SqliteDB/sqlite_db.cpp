#include "sqlite_db.h"
#include <Windows.h>

using namespace sqlite;

/*********************
*	Connection
*********************/
Connection::Connection()
{
}

void Connection::Open(std::string databaseName)
{
	sqlite3_initialize();
	int rc = sqlite3_open_v2(databaseName.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}
	OutputDebugString(L"Connessione aperta\n");
}

void Connection::Close()
{
	int rc = sqlite3_close_v2(db_);

	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}

	db_ = nullptr;
	OutputDebugString(L"Connessione chiusa\n");
}

int Connection::Execute(std::string command)
{
	//mutex.lock();
	sqlite3_stmt * stmt;
	int rc = sqlite3_prepare_v2(db_, command.c_str(), command.size(), &stmt, nullptr);

	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}

	int errcode = sqlite3_step(stmt);

	if (SQLITE_DONE != errcode)
	{
		throw SqliteException(sqlite3_errstr(errcode));
	}

	int rf = sqlite3_finalize(stmt);

	if (SQLITE_OK != rf)
	{
		throw SqliteException(sqlite3_errstr(rf));
	}

	return sqlite3_changes(db_);
	//mutex.unlock();
}

//Cursor Connection::ExecuteQuery(std::string query)
//{
//	/*sqlite3_stmt * stmt;
//	int rc = sqlite3_prepare_v2(db_, query.c_str(), query.size(), &stmt, nullptr);
//
//	if (SQLITE_OK != rc)
//	{
//	throw SqliteException(sqlite3_errstr(rc));
//	}*/
//
//	return Cursor(db_, query);
//}

long long int Connection::GetLastInsertRowid()
{
	return sqlite3_last_insert_rowid(db_);
}

Transaction Connection::BeginTransaction()
{
	int rc = sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}

	return Transaction{ db_ };
}

Connection::~Connection()
{
	Close();
}

/*******************
*	Cursor
********************/

Cursor::Cursor()
{

}

Cursor::Cursor(sqlite3 * db, std::string query) : db_{ db }
{
	int rc = sqlite3_prepare_v2(db_, query.c_str(), query.size(), &stmt_, nullptr);

	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}
}

Cursor::Cursor(sqlite3_stmt * stmt) : stmt_{ stmt }
{
}

bool Cursor::Next()
{
	wchar_t wstr[50];
	swprintf_s(wstr, 50, L"\t\tCursor::Next, puntatore stmt_=%p\n", stmt_);
	OutputDebugString(wstr);

	auto rc = sqlite3_step(stmt_);
	return rc == SQLITE_ROW;
}

int Cursor::GetInt(int columnIndex)
{
	return sqlite3_column_int(stmt_, columnIndex);
}

std::string Cursor::GetText(int columnIndex)
{
	return std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt_, columnIndex)));
}

double Cursor::GetDouble(int columnIndex)
{
	return sqlite3_column_double(stmt_, columnIndex);
}

Cursor::~Cursor()
{
	sqlite3_reset(stmt_);
	sqlite3_finalize(stmt_);
	stmt_ = nullptr;
}

/*********************
*	Transaction
*********************/
Transaction::Transaction(sqlite3 * db) : db_(db)
{
	pending = true;
}

int Transaction::Execute(std::string command)
{
	sqlite3_stmt * stmt;
	int rc = sqlite3_prepare_v2(db_, command.c_str(), command.size(), &stmt, nullptr);

	if (SQLITE_OK != rc)
	{
		throw SqliteException(sqlite3_errstr(rc));
	}

	int errcode = sqlite3_step(stmt);

	if (SQLITE_DONE != errcode)
	{
		throw SqliteException(sqlite3_errstr(errcode));
	}

	int rf = sqlite3_finalize(stmt);

	if (SQLITE_OK != rf)
	{
		throw SqliteException(sqlite3_errstr(rf));
	}

	return sqlite3_changes(db_);
}

long long int Transaction::GetLastInsertRowid()
{
	return sqlite3_last_insert_rowid(db_);
}

void Transaction::Commit()
{
	sqlite3_exec(db_, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
	pending = false;
}

void Transaction::Rollback()
{
	sqlite3_exec(db_, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr);
	pending = false;
}

Transaction::~Transaction()
{
	if (pending)
		Rollback();
}