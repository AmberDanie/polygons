//1. Вводится число прямых n
//2. Создается файл с генерируемыми прямыми random(0, +-3n)
//3. Вывести количество всех многоугольников (сколько 3угольникков, 4-ых и т.д.)
//4. Вывести какие это фигуры по точкам, все фигуры - выгнутые.

#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

// MUTEX ДЛЯ ОГРАНИЧЕНИЯ ДОСТУПА ПОТОКОВ -------------------------------------------------------------------< 

recursive_mutex rm;

// ФУНКЦИЯ ДЛЯ НАХОЖДЕНИЯ ТОЧКИ ПЕРЕСЕЧЕНИЯ ----------------------------------------------------------------<

float intersection_point(int k1, int k2, int b1, int b2) {
	return ((float)(-b1 + b2)) / ((float)(k1 + -k2)); // По формуле находим точку пересечения прямых.
}

// ФУНКЦИЯ-АНАЛОГ PYTHON SPLIT() ---------------------------------------------------------------------------<

void split(const string & str, vector<string> & tokens, const string & delimiters = ">")
{
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos); // Разделяем строку по элементу ">"
		pos = str.find_first_of(delimiters, lastPos);    // и заносим в созданный вектор токенов.
	}
}

// ФУНКЦИЯ ПРОВЕРКИ НА ВЫПУКЛОСТЬ --------------------------------------------------------------------------<

bool is_convex(vector<pair<float, float>> points) {
	pair<float, float> ab, bc;
	float product;
	int count = 0;
	bool flag = true;

	for (int i = 1; i < points.size() - 1; i++) {
		ab = {
			points[i].first - points[i - 1].first,
			points[i].second - points[i - 1].second
		};

		bc = {
			points[i + 1].first - points[i].first,
			points[i + 1].second - points[i].second
		};

		product = ab.first * bc.second - ab.second * bc.first; // Определяем направлении кривой.

		if (count == 0) {
			if (product > 0)
				flag = true;
			else
				flag = false;
			count = 1;
		}
		else {
			if (product > 0) {
				if (flag == false) {
					return false; // В случае, если направление изменилось, возвращаем false
				}
			}
			else {
				if (flag == true) {
					return false; // Здесь аналогично
				}
			}
		}
		if (product > 0)
			flag = true;
		else
			flag = false;
	}

	return true; // Если же фигура все время сворачивала в одном направлении, возвращаем true
}

// ФУНКЦИЯ ЗАПОЛНЕНИЯ ДВУХ ФАЙЛОВ ВСЕВОЗМОЖНЫМИ (ОДИНАРНЫМИ ВИДА X-Y) ПЕРЕСЕЧЕНИЯМИ ------------------------<

void first_intersection(int start, int end, string path, int* array_of_k)
{
	ifstream fin;
	ofstream fout;
	fin.seekg(0, ios::beg);
	fout.open(path);
	fin.open("intersections.csv");
	string trash, str1 = "";
	float angle = 0;
	for (int i = start; i < end; i += 2) {
		string str = "";
		if (!fin.eof())
			getline(fin, str);
		if (str != "")
			if ((str.substr(0, str.find(":"))) != "")
				if ((stoi(str.substr(0, str.find(":"))) % 2) != start)
					getline(fin, str);
		int count = 0;
		int counter = 0;
		for (int j = (str.substr(0, str.find(':'))).length() + 1; j < str.length(); j++) {
			if (str[j] != ',') {
				if (count == 0) {
					fout << str.substr(0, str.find(':')) + '>'; // Если вхождение первое, то
					count += 1;                                // в файл заносим только 1 элемент
				}
				str1 += str[j];
			}

			else {
				fout << str1 << endl; // После заносим второй через разделитель ">".
				str1 = "";
				count = 0;
			}
		}
	}
	fout.close();
	fin.close();
}

// ГЕНЕТИЧЕСКАЯ ФУНКЦИЯ УВЕЛИЧЕНИЯ ЦЕПОЧКИ ПЕРЕСЕЧЕНИЙ НА МЕСТЕ (из X-Y-Z в X-Y-Z-W и т.п.) ----------------< 

int file_research(const char main_path[], const char main_transient_path[],
	const char second_transient_path[], int num_of_funcs, string * array_of_string, // тут жесть
	int num_of_chain, int* array_of_k, int* array_of_b, ofstream &fout1, ofstream &fout2) {
	ifstream fin;
	int count = 0, num_of_figures = 0;
	string last_str = "", str1 = "", cur_str = "";
	fin.open(main_path);
	fin.seekg(0, ios::beg);
	fin.close();
	fin.open(main_path);
	int counter = 0;
	while (getline(fin, str1)) { // Получаем строку вида X-Y-..-Z для преобразования в строку порядка выше. 
		if (str1 != "") {
			string str = array_of_string[stoi(str1.substr(str1.find_last_of(">") + 1,
				str1.find_last_of(" "))) - 1];
			for (int j = str.find(":") + 1; j < str.length(); j++) {
				if (str[j] != ',') {
					if (count == 0) {
						cur_str = str1.substr(0, str1.find_last_of(" ")) + '>';
						count += 1;
					}
					cur_str += str[j];
					last_str += str[j];
				}

				else {
					if (count != 0) {
						vector <string> vector_of_str;
						vector <pair<float, float>> vector_of_inter;
						split(cur_str, vector_of_str); // Разбиваем цепочку на составляющие
													  // и записываем в вектор (аналог питоновского сплита)

						for (int i = 0; i < vector_of_str.size() - 1; i++) {
							if (vector_of_str[i] != "") {
								float x_of_inter = intersection_point(array_of_k[stoi(vector_of_str[i]) - 1],
									array_of_k[stoi(vector_of_str[i + 1]) - 1], array_of_b[stoi(vector_of_str[i]) - 1],
									array_of_b[stoi(vector_of_str[i + 1]) - 1]);
								float y_of_inter = array_of_k[stoi(vector_of_str[i]) - 1] * x_of_inter +
									array_of_b[stoi(vector_of_str[i]) - 1];
								vector_of_inter.push_back(make_pair(x_of_inter, y_of_inter));
								// Выше находим координаты пересечений и записываем попарно в вектор пересечений
							}
						}

						if (is_convex(vector_of_inter)) { 
							if (counter % 2 == 0)								// Проверяем функцией, выпуклая ли текущая ломанная линия.
								fout1 << cur_str + '\n';
							else
								fout2 << cur_str + '\n';					// Если уже на текущем этапе она невыпукла 
																		   // (изменилось направление ломанной),
																		  // то не будем дальше с ней работать
							if (vector_of_str[vector_of_str.size() - 1] != "") {
								float x_of_inter = intersection_point(array_of_k[stoi(vector_of_str[0]) - 1],
									array_of_k[stoi(vector_of_str[vector_of_str.size() - 1]) - 1], array_of_b[stoi(vector_of_str[0]) - 1],
									array_of_b[stoi(vector_of_str[vector_of_str.size() - 1]) - 1]);
								float y_of_inter = array_of_k[stoi(vector_of_str[0]) - 1] * x_of_inter +
									array_of_b[stoi(vector_of_str[0]) - 1];

								vector_of_inter.push_back(make_pair(x_of_inter, y_of_inter));
																   // Здесь проверяем, выпуклая ли в целом фигура на данном этапе.
							}
							if (is_convex(vector_of_inter)) {	 // Если выше получили, что такая фигура выпуклая, 
																// и при этом пересечение существует, записываем.
								if ((array_of_string[stoi(str1.substr(0, str1.find(":"))) - 1].find(last_str) != -1)) {
									num_of_figures += 1; // +1 к текущим фигурам
								}
							}
						}
						count = 0;
						counter += 1;
						last_str = "";
						vector_of_str.clear();	  // Очищаем 
						vector_of_inter.clear(); // вектор
					}
				}
			}
		}
	}
	return num_of_figures; // Возвращаем количество текущих i-угольников
}

// ОСНОВА --------------------------------------------------------------------------------------------------<

int main() {

	srand(time(NULL)); // Устанавливаем в качестве рандомайзера текущее время, 
					  // чтобы программа каждый раз гененировала новые значения.

	ios_base::sync_with_stdio(false);	// Убираем задержку для вывода и ввода,
	cin.tie(0);						   //  чтобы не тратить время на вывод
	cout.tie(0);

	setlocale(LC_ALL, "Rus");

	int n = 0;

	// ВВОД КОЛИЧЕСТВА ПРЯМЫХ n ----------------------------------------------------------------------------<

	cout << "Введите количество прямых n: ";
	while (!(cin >> n) or (n <= 0) or (getchar() != '\n')) {  // Стандартный ввод с ограничениями на int-овость и положительность
		cin.clear();
		cin.ignore(10000000, '\n');
		cout << "\nНекорретный ввод. Попробуйте еще раз";
		cout << "\nВведите количество прямых n: ";
	}
	cout << to_string(n) + " случайно заданных прямых:" << endl;
	unsigned int start_time = clock();

	/// ДИН. МАССИВЫ --------------------------------------------------------------------------------------->

	int* array_of_k = new int[n];   // Массив коэффицентов k перед иксом. k соотв. tang угла, 
								   // под которым направлена прямая в положительном направлении оси OY
	int* array_of_b = new int[n]; // Массив коэффицентов b. 

	// ЗАПИСЬ В ФАЙЛ ---------------------------------------------------------------------------------------<

	ofstream fout;
	fout.open("functions.txt");

	string func = "";
	int znak = 0;
	char znak_b = ' ';
	int k, b = 0;

	for (int i = 0; i < n; i++) {
		if (rand() % 2 == 0)
			znak = 1;
		else
			znak = -1;
		k = znak * rand() % (3 * n + 1);  // Гененируем значение для k, соотв. произведению знака, 
										 // генерируемого с шансом 1/2 и числа, 
										// находящегося в диапазоне от 0 до 3*n. 
									   // В результате k принадл. диапазону [-3n; 3n].
		if (rand() % 2 == 0) {
			znak = 1;
			znak_b = '+';
		}
		else {
			znak = -1;
			znak_b = '-';
		}
		b = znak * rand() % (3 * n + 1);  // Для b все аналогично коэффиценту k. Знак генерируется заново, 
										 // иначе знаки у k и b будут всегда совпадать.
		func = "y = ";					// y = kx + b.
		if (k == 0 and b == 0)
			func = func + to_string(0); // Если и k, и b получились равными 0, то выводим y = 0.
		else {						   // Иначе:
			if (k != 0)
				func = func + to_string(k) + "x ";   // Если k != 0, то добавляем к "y = " значение kx. 
													// Иначе этот шаг пропускаем.
			if (b != 0) {						   // Аналогично с b. Для красоты и
												  // удобства знак отделяем от числа через пробел, 
												 //	а само b записываем по модулю (abs)
				func = func + znak_b;
				if (k != 0)
					func = func + " ";
				func = func + to_string(abs(b));
			}
		}
		cout << to_string(i + 1) + ") " << func << endl;   // Выводим получившееся уравнение вида y = kx + b. 
														  // Для ускорения процесса можно закомментировать*.
		fout << to_string(i + 1) + ") " << func << "\n"; // Вводим функцию в файл, из которого потом будем 
														// считывать.
	}

	fout.close();

	// СЧИТЫВАНИЕ ФАЙЛА ------------------------------------------------------------------------------------<

	ifstream fin;
	string str, str1 = "";
	fin.open("functions.txt");
	int count = 0;
	while (getline(fin, str)) { // Построчно читаем файл и обрабатываем каждую строку 
		str1 = "";             // для записи в массив коэффицентов k и b.
		if (str.find('x') != -1) {
			for (int i = str.find('=') + 1; i < str.find('x'); i++) {
				if (str[i] != ' ' and str[i] != '=' and str[i] != 'y')
					str1 += str[i];
			}
		}
		else {
			str1 = '0'; // Если в строке отстутствует x (т.е. k = 0), то строке присваеваем 0. 
					   // Иначе выполняется цикл выше, где "вычленяется" все 
					  // между "=" и "x" с пробельным сплитом (пробелы игнорятся)
		}

		array_of_k[count] = stoi(str1);   // В массив k коэффицентов записывается полученная выше строка, 
										 // переведенная в int (т.к. все значения - целочисленные).

		str1 = "";
		if (str.find('x') != -1) {	   // Аналогично делаем с b, только с некоторыми усложенинями. 
									  // Если x отсутствует в строке, то b ищется от "="
									 // до конца строки, иначе от индекса x + 1 и до конца.
									// Если в результате одного из циклов строка 
								   // остается пустой, то в строку записывается 0.

			for (int i = str.find('x') + 1; i < str.length(); i++) {
				if (str[i] != ' ' and str[i] != '=' and str[i] != 'y')
					str1 += str[i];
			}
		}
		else {
			for (int i = str.find('='); i < str.length(); i++) {
				if (str[i] != '=' and str[i] != ' ')
					str1 += str[i];
			}
		}
		if (str1 == "")
			str1 = '0';
		array_of_b[count] = stoi(str1);  // Записываем значение b в массив b-шек.
		count += 1;						// Переходим к следующей прямой, увеличив счетчик.
	}

	fin.close();

	// ЗАПОЛНЕНИЕ CSV-ФАЙЛА ПЕРЕСЕЧЕНИЙ --------------------------------------------------------------------<

	fout.open("intersections.csv");
	int counter = 0; float angle = 0;
	float x_of_inter = 0, y_of_inter = 0;
	for (int i = 0; i < n - 1; i++) {
		fout << to_string(i + 1) + ":"; // Устанавливаем в значении ключа индекс i
		for (int j = i + 1; j < n; j++) {
			if ((array_of_k[i] - array_of_k[j]) != 0) {
				if (j != n)
					fout << to_string(j + 1) + ","; // Если прямая j пересекается с i, то записываем
			}
		}
		fout << "\n";
	}
	fout.close();

	string* array_of_string = new string[n]{};
	fin.open("intersections.csv");
	for (int i = 0; i < n - 1; i++) {
		getline(fin, str);
		array_of_string[i] = str;
	}
	fin.close();

	// ЗАПОЛНЕНИЕ ДВУХ НАЧАЛЬНЫХ ФАЙЛОВ ОДИНОЧНЫМИ ПЕРЕСЕЧЕНИЯМИ ВИДА X-Y (ПОТОК) --------------------------<

	fout.open("second-half.txt"); // Создаем файл #2
	fout.close();
	fout.open("first-half.txt"); // Создаем файл #1
	fout.close();
	first_intersection(0, n, "second-half.txt", array_of_k); // Заполняем файл №2 одной половиной пересечений
	first_intersection(1, n, "first-half.txt", array_of_k); // и файл №1 соотв. другой половиной

	// -----------------------------------------------------------------------------------------------------<

	fout.open("transient_file_for_first.txt"); // Создаем временный файл для обработки файла #2
	fout.close();
	fout.open("transient_file_for_second.txt"); // И соотв. временный файл для обработки файла #1
	fout.close();
	fout.open("figures.txt"); // Также создаем файл фигур для контроля правильности выполнения
	fout.close();

	// ПРОГОНЯЕМ АЛГОРИТМ, УВЕЛИЧИВАЮЩИЙ ЦЕПЬ ЗАМЫКАЕМЫХ СИМВОЛОВ ДЛЯ НАХОЖДЕНИЯ ФИГУР ---------------------<

	for (int i = 0; i < n - 2; i++) { // Главный цикл. В нем на каждом шаге, прогоняя функцию file_research,
									 // увеличиваем цепочку на 1 и одновременно проверяем кривую линии на
									// выпуклость. Вроде как это называется генетический алгоритм, но
								   // я не уверен.
		ofstream fout1;
		ofstream fout2;
		fout1.open("transient_file_for_first.txt"); fout.clear(); 
		fout2.open("transient_file_for_second.txt"); fout.clear(); 
		int num_of_figures_cur = 0;
		bool flag = false;
		thread thstream( // В одном потоке гоняем файл #1
			[n, array_of_string, i, &flag, &num_of_figures_cur, array_of_k, array_of_b, &fout1, &fout2]() {

				num_of_figures_cur += file_research("second-half.txt",
					"transient_file_for_second.txt", "transient_file_for_first.txt",
					n, array_of_string, i, array_of_k, array_of_b, fout1, fout2);

				flag = true;
				return flag;
			});
		thstream.detach();

		num_of_figures_cur += file_research("first-half.txt",
			"transient_file_for_first.txt", "transient_file_for_second.txt",
			n, array_of_string, i, array_of_k, array_of_b, fout1, fout2); // Сама программа гоняет файл #2

		while (!(flag == true)) { // На случай, если созданный поток thstream отстал от основного,
			continue;            // делаем паузу в вайле пока оба потока не завершат обработку
		}
		fout1.close(); fout2.close();
		int result = 0;

		result = remove("first-half.txt");   // осле срабатывания шага алгоритма удаляем файл #1
		result = remove("second-half.txt"); // и соотв. файл #2

		result = rename("transient_file_for_first.txt", "first-half.txt");    // И заменяем временные файлы 
		result = rename("transient_file_for_second.txt", "second-half.txt"); // на основные

		fout.close();

		fout.open("figures.txt", ios::app);

		if (num_of_figures_cur != 0)
			fout << num_of_figures_cur << " <" << i + 3 << ">угольников" << endl; // Записываем количество 
																				 // текущих фигур
		fout.close();
	}

	cout << ">--------------<" << endl;

	fin.open("figures.txt"); // Выводим все фигуры в порядке возрастания
	while (getline(fin, str))
		cout << str << endl;
	fin.close();

	cout << ">--------------<" << endl;

	// УДАЛЕНИЕ ДИНАМИЧЕСКИХ МАССИВОВ  ---------------------------------------------------------------------<

	delete[] array_of_k;
	delete[] array_of_b;
	delete[] array_of_string;

	unsigned int end_time = clock(); // Время завершения программы

	cout << "Время работы программы: " << end_time - start_time << " мс." << endl; // выв. время выполнения
	return 0;
}