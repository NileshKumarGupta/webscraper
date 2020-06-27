#include <iostream>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
// compile with g++ -o db db.cpp `pkg-config --libs --cflags libmongocxx`
// -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -L/usr/local/lib -lmongocxx -lbsoncxx

int main(int, char **)
{
  mongocxx::instance inst{};
  mongocxx::uri uri(mongouri);
  mongocxx::client client(uri);

  bsoncxx::builder::stream::document document{};

  mongocxx::database db = client["webscraper"];
  mongocxx::collection coll = db["paras"];
  document << "hello"
           << "suprabhat";

  coll.insert_one(document.view());
  auto cursor = coll.find({});

  for (auto &&doc : cursor)
  {
    std::cout << bsoncxx::to_json(doc) << std::endl;
  }
}