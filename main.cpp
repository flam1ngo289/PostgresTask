#include <iostream>
#include <pqxx/pqxx>

class ClientManager {
public:
    ClientManager(const std::string& connectionString) : conn(connectionString) {}

    void createDatabaseStructure() {
        pqxx::work txn(conn);
        txn.exec("CREATE TABLE IF NOT EXISTS clients ("
            "id SERIAL PRIMARY KEY, "
            "first_name VARCHAR(50), "
            "last_name VARCHAR(50), "
            "email VARCHAR(100));");

        txn.exec("CREATE TABLE IF NOT EXISTS phones ("
            "id SERIAL PRIMARY KEY, "
            "client_id INTEGER REFERENCES clients(id) ON DELETE CASCADE, "
            "phone_number VARCHAR(20));");
        txn.commit();
    }

    void addClient(const std::string& first_name, const std::string& last_name, const std::string& email) {
        pqxx::work txn(conn);
        txn.exec_params("INSERT INTO clients (first_name, last_name, email) VALUES ($1, $2, $3);", first_name, last_name, email);
        txn.commit();
    }

    void addPhone(int client_id, const std::string& phone_number) {
        pqxx::work txn(conn);
        txn.exec_params("INSERT INTO phones (client_id, phone_number) VALUES ($1, $2);", client_id, phone_number);
        txn.commit();
    }

    void updateClient(int client_id, const std::string& first_name, const std::string& last_name, const std::string& email) {
        pqxx::work txn(conn);
        txn.exec_params("UPDATE clients SET first_name=$1, last_name=$2, email=$3 WHERE id=$4;", first_name, last_name, email, client_id);
        txn.commit();
    }

    void deletePhone(int phone_id) {
        pqxx::work txn(conn);
        txn.exec_params("DELETE FROM phones WHERE id=$1;", phone_id);
        txn.commit();
    }

    void deleteClient(int client_id) {
        pqxx::work txn(conn);
        txn.exec_params("DELETE FROM clients WHERE id=$1;", client_id);
        txn.commit();
    }

    std::vector<std::tuple<int, std::string, std::string, std::string>> findClient(const std::string& search_term) {
        pqxx::work txn(conn);
        pqxx::result result = txn.exec_params(
            "SELECT id, first_name, last_name, email FROM clients "
            "WHERE first_name ILIKE $1 OR last_name ILIKE $1 OR email ILIKE $1;",
            "%" + txn.esc(search_term) + "%");

        std::vector<std::tuple<int, std::string, std::string, std::string>> clients;
        for (auto row : result) {
            clients.emplace_back(row[0].as<int>(), row[1].as<std::string>(), row[2].as<std::string>(), row[3].as<std::string>());
        }
        return clients;
    }

private:
    pqxx::connection conn;
};

int main(){
    try {

	std::string connectionString = 
		"host=127.0.0.1 "
		"port=5432 "
		"dbname=postgres "
		"user=postgres "
		"password=postgres ";

	pqxx::connection conn(connectionString);

    ClientManager manager(connectionString);

    manager.createDatabaseStructure();

    // Добавление клиента
    manager.addClient("Ivan", "Ivanov", "ivan.ivanov@mail.ru");

    // Поиск клиента
    auto clients = manager.findClient("Ivan");
    for (const auto& client : clients) {
        std::cout << "Found client: " << std::get<1>(client) << " " << std::get<2>(client) << " " << std::get<3>(client) << std::endl;
    }

    // Добавление телефона для клиента
    if (!clients.empty()) {
        int client_id = std::get<0>(clients[0]);
        manager.addPhone(client_id, "123-456-7890");
    }

    // Изменение данных о клиенте
    if (!clients.empty()) {
        int client_id = std::get<0>(clients[0]);
        manager.updateClient(client_id, "Ivan", "Smith", "ivan.smith@mail.ru");
    }

    // Удаление телефона
    // manager.deletePhone(phone_id); // Укажите phone_id для удаления

    // Удаление клиента
    if (!clients.empty()) {
        int client_id = std::get<0>(clients[0]);
        manager.deleteClient(client_id);
    }

    }  catch (const pqxx::sql_error& e) {
        std::cerr << "Ошибка SQL: " << e.what() << std::endl;
        std::cerr << "Был запрос: " << e.query() << std::endl;
        return 1;
    }
    catch (const pqxx::broken_connection& e) {
        std::cerr << "Ошибка соединения: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;

}