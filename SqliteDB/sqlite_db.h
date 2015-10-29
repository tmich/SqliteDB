#pragma once

#include "sqlite3.h"
#include <exception>
#include <string>

namespace sqlite
{
	class SqliteException : public std::exception
	{
	public:
		SqliteException(std::string message) : message_(message) {};

		virtual const char * what() const override
		{
			return message_.c_str();
		}
	protected:
		std::string message_;
	};

	class Cursor
	{
	public:
		Cursor();
		Cursor(sqlite3_stmt * stmt);
		~Cursor();

		bool Next();
		int GetInt(int columnIndex);
		std::string GetText(int columnIndex);
	private:
		sqlite3_stmt * stmt_;
	};

	class Transaction
	{
	public:
		Transaction(sqlite3 * db);
		~Transaction();

		void Commit();
		void Rollback();
		int Execute(std::string command);
	protected:
		bool pending;
		sqlite3 * db_;
	};

	class Connection
	{
	public:
		Connection();
		~Connection();

		void Open(std::string databaseName);
		void Close();

		/* Executes NON-QUERY command */
		int Execute(std::string command);

		Cursor ExecuteQuery(std::string query);

		Transaction BeginTransaction();
	protected:
		sqlite3 * db_{ nullptr };
	};
}