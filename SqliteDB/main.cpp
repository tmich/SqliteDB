#include "sqlite_db.h"
#include <string>
#include <vector>
#include <iostream>

using namespace sqlite;

struct Album
{
	int Id;
	std::string Title;
	int Year;
};

int main(char ** argv, int argc)
{
	Connection conn;
	conn.Open("my.db");
	conn.Execute("create table if not exists album (id integer primary key, title text, year int);");
	auto trn = conn.BeginTransaction();
	trn.Execute("insert into album (title, year) values ('Un biglietto del tram', 1975);");
	trn.Execute("insert into album (title, year) values ('Storia di un minuto', 1973);");
	trn.Execute("insert into album (title, year) values ('The lamb lies down on Broadway', 1974);");
	trn.Commit();

	auto crs = conn.ExecuteQuery("select id, title, year from album;");

	std::vector<Album> albums;

	while (crs.Next())
	{
		Album album;
		album.Id = crs.GetInt(0);
		album.Title = crs.GetText(1);
		album.Year = crs.GetInt(2);

		albums.push_back(album);
	}

	for (auto & alb : albums)
	{
		std::cout << "ID: " << alb.Id << "\tTitle: " << alb.Title << "\tYear:" << alb.Year << std::endl;
	}

	conn.Execute("delete from album where id = 5");
}