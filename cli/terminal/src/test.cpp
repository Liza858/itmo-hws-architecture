/*
 * Ivan Rybin, 2020.
 * Software Architecture, ITMO JB SE.
 *
 * Набор тестов для проверки функциональности terminal.
 */
#include "terminal.hpp"

#include <cassert>
#include <string>
#include <sstream>

// UNCOMMENT TO ENABLE TESTS
#define ECHO_CMD
#define CAT_CMD
#define PWD_CMD
#define WC_CMD
#define GREP_CMD
#define PIPE

namespace fs = std::experimental::filesystem;

static void clear_streams(std::stringstream& fst, std::stringstream& snd, std::stringstream& thd) {
    fst.str(""); fst.clear();
    snd.str(""); snd.clear();
    thd.str(""); thd.clear();
}

static void gl(std::stringstream& sout, std::string& out) {
    getline(sout, out);
}

static void echo_cmd_and_dollar_test() {
#ifdef ECHO_CMD
    std::string l;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics
    {
        sin << "echo"                   << "\n";
        sin << "echo   1    2      3"   << "\n";
        sin << "echo \"4  5  6\""       << "\n";
        sin << "echo \'a  \"b\"  c\'"   << "\n";
        sin << "echo \"x  \'y\'  z\""   << "\n";

        test_t.run();

        gl(sout, l); assert("" == l);
        gl(sout, l); assert("1 2 3" == l);
        gl(sout, l); assert("4  5  6" == l);
        gl(sout, l); assert("a  \"b\"  c" == l);
        gl(sout, l); assert("x  \'y\'  z" == l);
        clear_streams(sin, sout, serr);
    }
    // dollars
    {
        sin << "x=15" << "\n";
        sin << "t=\"test\""     << "\n";
        sin << "e=\'echo\'"     << "\n";

        sin << "echo $x"        << "\n";
        sin << "$e $t"          << "\n";
        sin << "echo \"$x\""    << "\n";
        sin << "echo \'$x\'"    << "\n";

        test_t.run();

        gl(sout, l); assert("15" == l);
        gl(sout, l); assert("test" == l);
        gl(sout, l); assert("15" == l);
        gl(sout, l); assert("$x" == l);
        clear_streams(sin, sout, serr);
    }
    // errors
    {
        sin << "x= 15"        << "\n";
        sin << "t =15"        << "\n";
        sin << "e=test echo"  << "\n";

        test_t.run();

        gl(serr, l); assert("x: команда не найдена" == l);
        gl(serr, l); assert("t: команда не найдена" == l);
        gl(serr, l); assert("echo: команда не найдена" == l);
        clear_streams(sin, sout, serr);
    }
#endif
}

static void cat_cmd_test() {
#ifdef CAT_CMD
    std::string l;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics
    {
        std::ofstream file("cat_test.txt", std::ofstream::out | std::ofstream::trunc);
        file << "test cat\n"
                "multiple string\n"
                "with 3 lines";
        file.close();

        sin << "cat cat_test.txt" << "\n";

        test_t.run();
        assert("test cat\nmultiple string\nwith 3 lines\n" == sout.str());
        clear_streams(sin, sout, serr);

        sin << "cat "
               "cat_test.txt "
               "cat_test.txt "
               "cat_test.txt" << "\n";
        test_t.run();
        assert("test cat\nmultiple string\nwith 3 lines\n"
               "test cat\nmultiple string\nwith 3 lines\n"
               "test cat\nmultiple string\nwith 3 lines\n" == sout.str());
        clear_streams(sin, sout, serr);
    }
    // errors
    {
        sin << "cat abracadabra.hs" << "\n";

        test_t.run();
        assert("cat: abracadabra.hs: Нет такого файла или каталога\n" == serr.str());
        clear_streams(sin, sout, serr);

    }
#endif
}

static void pwd_cmd_test() {
#ifdef PWD_CMD
    std::string l;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics
    {
        sin << "pwd" << "\n";
        sin << "pwd 1 2 3" << "\n";
        test_t.run();

        gl(sout, l); assert(fs::current_path() == l);
        gl(sout, l); assert(fs::current_path() == l);
        clear_streams(sin, sout, serr);
    }
#endif
}

static void wc_cmd_test() {
#ifdef WC_CMD
    std::string l;
    size_t l_cnt  = 0;
    size_t w_cnt  = 0;
    size_t f_size = 0;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics - empty file
    {
        std::ofstream file("wc_test.txt", std::ofstream::out | std::ofstream::trunc);
        file.close();

        sin << "wc wc_test.txt" << "\n";
        test_t.run();

        sout >> l_cnt; sout >> w_cnt; sout >> f_size;
        assert(l_cnt == 0);
        assert(w_cnt == 0);
        assert(f_size == 0);
        clear_streams(sin, sout, serr);
    }

    // basics - not empty file
    {
        std::ofstream file("wc_test.txt", std::ofstream::out | std::ofstream::trunc);
        file << "abcdefghijklmn\n"
                "opqrstuvwxyz\n";
        file.close();

        sin << "wc wc_test.txt" << "\n";

        test_t.run();
        sout >> l_cnt; sout >> w_cnt; sout >> f_size;
        assert(l_cnt == 2);
        assert(w_cnt == 2);
        assert(f_size == 28); // 2 bytes for '\n'

        assert(" 2\t2\t28 wc_test.txt\n" == sout.str());
        clear_streams(sin, sout, serr);
    }

    // basics - many files
    {
        std::ofstream file("wc_test.txt", std::ofstream::out | std::ofstream::trunc);
        file << "abcdefghijklmn\n"
                "opqrstuvwxyz\n";
        file.close();

        sin << "wc wc_test.txt wc_test.txt wc_test.txt" << "\n";

        test_t.run();
        gl(sout, l); assert(" 2\t2\t28 wc_test.txt" == l);
        gl(sout, l); assert(" 2\t2\t28 wc_test.txt" == l);
        gl(sout, l); assert(" 2\t2\t28 wc_test.txt" == l);
        gl(sout, l); assert(" 6\t6\t84 итого" == l);
        clear_streams(sin, sout, serr);
    }
    // errors
    {
        sin << "wc abc.xyz" << "\n";
        test_t.run();
        assert("wc: abc.xyz: Нет такого файла или каталога\n" == serr.str());
    }
#endif
}

static void grep_cmd_test() {
#ifdef GREP_CMD
    std::string l;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics [-w] [-A n] [-i]
    {
        std::ofstream file("grep_test.txt", std::ofstream::out | std::ofstream::trunc);
        file << "grep\n"
                "Line №2 GREPTEST\n"
                "Line №3 abcdefgh\n"
                "Line №4 ABC word\n"
                "end of file\n";
        file.close();

        sin << "grep grep grep_test.txt" << "\n";       // just pattern
        test_t.run();

        gl(sout, l); assert("grep" == l);
        clear_streams(sin, sout, serr);


        sin << "grep -i grep grep_test.txt" << "\n";    // [-i] non case sensitive
        test_t.run();

        assert("grep\n"
               "Line №2 GREPTEST\n" == sout.str());
        clear_streams(sin, sout, serr);


        sin << "grep -wi abc grep_test.txt" << "\n";    // [-w] only words
        test_t.run();

        assert("Line №4 ABC word\n" == sout.str());
        clear_streams(sin, sout, serr);


        sin << "grep -wA2 grep grep_test.txt" << "\n";  // [-A n] n lines after matching
        test_t.run();

        assert("grep\n"
               "Line №2 GREPTEST\n"
               "Line №3 abcdefgh\n" == sout.str());
        clear_streams(sin, sout, serr);


        sin << "grep -w grep grep_test.txt grep_test.txt grep_test.txt" << "\n"; // many files
        test_t.run();

        assert("grep_test.txt:grep\n"
               "grep_test.txt:grep\n"
               "grep_test.txt:grep\n" == sout.str());
        clear_streams(sin, sout, serr);

        std::remove("grep_test.txt");
    }
    // errors
    {
        sin << "grep pattern abc.xyz" << "\n";
        test_t.run();
        assert("grep: abc.xyz: Нет такого файла или каталога\n" == serr.str());
        clear_streams(sin, sout, serr);

        sin << "grep -A pattern abc.xyz" << "\n";
        test_t.run();
        assert("grep: bad_context: Неверный аргумент длины контекста\n" == serr.str());
        clear_streams(sin, sout, serr);

        sin << "grep -x pattern abc.xyz" << "\n";
        test_t.run();
        assert("grep: -x: Неверный ключ\n" == serr.str());
        clear_streams(sin, sout, serr);
    }
#endif
}

static void pipe_test() {
#ifdef PIPE
    std::string l;
    std::stringstream sin{};
    std::stringstream sout{};
    std::stringstream serr{};
    terminal test_t(sin, sout, serr, true);

    // basics
    {
        sin << "echo 1 2 3 | wc"         << "\n";
        sin << "echo 1 | echo 2"         << "\n";
        sin << "echo 1 | echo 2 | pwd "  << "\n";

        test_t.run();

        gl(sout, l); assert(" 1\t3\t6" == l);
        gl(sout, l); assert("2" == l);
        gl(sout, l); assert(fs::current_path() == l);
    }

#endif
}

int main() {
    bool is_exc = false;
    try {
        echo_cmd_and_dollar_test();
        cat_cmd_test();
        pwd_cmd_test();
        wc_cmd_test();
        grep_cmd_test();
        pipe_test();
    } catch (std::exception& e) {
        is_exc = true;
        std::cout << "Oh, something bad happened:" << std::endl;
        std::cout << e.what() << std::endl;
    }
    if (!is_exc) {
        std::cout << "6/6 Tests passed!" << std::endl;
    }
    return 0;
}
