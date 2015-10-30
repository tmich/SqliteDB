#include "sqlite_db.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <thread>

using namespace sqlite;

struct Album
{
	int Id;
	std::string Title;
	int Year;
	double Price;
};

int main(char ** argv, int argc)
{
	Connection conn;
	conn.Open("my.db");
	conn.Execute("create table if not exists album (id integer primary key, title text, year int, price double);");
	
	/*auto trn = conn.BeginTransaction();
	try
	{
		trn.Execute("insert into album (title, year, price) values ('Un biglietto del tram', 1975, 9.90);");
		std::cout << "Ultimo id inserito: " << conn.GetLastInsertRowid() << std::endl;
		trn.Execute("insert into album (title, year, price) values ('Storia di un minuto', 1973, 8.80);");
		std::cout << "Ultimo id inserito: " << trn.GetLastInsertRowid() << std::endl;
		trn.Execute("insert into album (title, year, price) values ('The lamb lies down on Broadway', 1974, 12);");
		std::cout << "Ultimo id inserito: " << trn.GetLastInsertRowid() << std::endl;
		trn.Commit();
	}
	catch (SqliteException& exc)
	{ 
		std::cout << "*** ERRORE: " << exc.what() << " ***" << std::endl;
		trn.Rollback();
		std::terminate();
	}*/

	/* inizio test multithreading */

	std::vector<std::thread> threads;
	for (int i = 0; i < 5; ++i){
		threads.push_back(std::thread([&conn](){
			for (int i = 0; i < 100; ++i){
				std::ostringstream insert;
				insert << "insert into album(title, year, price) values('Titolo " << i << "', 197" << i << ", " << i << ".00); ";
				OutputDebugStringA(insert.str().c_str()); OutputDebugStringA("\n");
				conn.Execute(insert.str());
			}
		}));
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
	/* fine test multithreading */


	Cursor crs(conn, std::string("select id, title, year, price from album;"));

	std::vector<Album> albums;

	while (crs.Next())
	{
		Album album;
		album.Id = crs.GetInt(0);
		album.Title = crs.GetText(1);
		album.Year = crs.GetInt(2);
		album.Price = crs.GetDouble(3);

		albums.push_back(album);
	}

	for (auto & alb : albums)
	{
		std::cout << "ID: " << alb.Id << "\tTitle: " << alb.Title << "\t\tYear: " << alb.Year << "\tPrice: " << alb.Price << std::endl;
	}

	//conn.Execute("delete from album where id = 5");
}