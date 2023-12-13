using System;
using Microsoft.Data.Sqlite;

namespace SharpDB
{
    class Program
    {
        static void Main(string[] args)
        {
            using (var db = new SqliteConnection("Data Source=db.sqlite"))
            {
                db.Open();
                var dbc = db.CreateCommand();



                while (true)
                {
                    Console.WriteLine("    Меню:\n");
                    Console.WriteLine("1. Вывести имена");
                    Console.WriteLine("2. Показать 1 в небезопасном варианте");
                    Console.WriteLine("3. Показать 1 в безопасном варианте");
                    Console.WriteLine("4. Поднять базу");
                    Console.WriteLine("0. Экзит");

                    int a = Convert.ToInt32(Console.ReadLine());
                    switch (a)
                    {
                        case 1:
                            {
                                dbc.CommandText = "SELECT fio FROM test";
                                dbc.ExecuteNonQuery();
                                SqliteDataReader reader = dbc.ExecuteReader(); //Для чтения данных из базы данных SQLite применяется метод 
                                while (reader.Read())
                                    Console.WriteLine("{0}", reader.GetString(0));
                                reader.Close();
                                break;
                            }
                        case 2:
                            {
                                Console.WriteLine("\nВведите имя:");
                                string name = Console.ReadLine();

                                Console.WriteLine("\nВведите балл:");
                                int ball = Convert.ToInt32(Console.ReadLine());

                                

                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, '{name}', {ball})";
                                dbc.ExecuteNonQuery();

                                /*SqliteDataReader reader = dbc.ExecuteReader();
                                while (reader.Read())
                                    Console.WriteLine("{0}", reader.GetString(0));
                                reader.Close();*/
                                break;
                            }
                        case 3:
                            {
                                Console.WriteLine("\nВведите имя:");
                                string name = Console.ReadLine();   

                                using (SqliteCommand command = new SqliteCommand("select ball from test where fio like @object", db))
                                {
                                    command.Parameters.AddWithValue("@object", name);
                                    var output = command.ExecuteScalar();
                                    if (output == null)
                                        Console.WriteLine("Нет такого юзера");
                                    else
                                        Console.WriteLine(output);
                                }
                                break;
                            }
                        case 4:
                            {
                                dbc.CommandText = $"DROP TABLE IF EXISTS test";
                                dbc.ExecuteNonQuery();

                                dbc.CommandText = @" CREATE TABLE IF NOT EXISTS test(
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    fio TEXT,
                    ball INTEGER)";
                                dbc.ExecuteNonQuery();


                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, 'Алешко Альберт', {80})";
                                dbc.ExecuteNonQuery();
                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, 'Негробов Виктор', {100})";
                                dbc.ExecuteNonQuery();
                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, 'Галимов Айдар', {60})";
                                dbc.ExecuteNonQuery();
                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, 'Батюшкова Мария', {75})";
                                dbc.ExecuteNonQuery();
                                dbc.CommandText = $"INSERT INTO test VALUES (NULL, 'Костенко Егор', {90})";
                                dbc.ExecuteNonQuery();


                                break;
                            }
                        default:
                            { return; }
                    }
                }
            }
        }
    }
}
