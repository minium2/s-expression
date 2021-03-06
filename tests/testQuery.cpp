#define lest_FEATURE_AUTO_REGISTER 1
#include <detail/Lexer.h>
#include <SDocument.h>
#include "lest.hpp"

#define CASE( name ) lest_CASE( testlist, name )
#undef SCENARIO
#define SCENARIO( sketch ) lest_SCENARIO(testlist, sketch )

static lest::tests testlist;

SCENARIO("trivial s-document") {
    GIVEN("parse trivial string") {
        const auto str = "(a1 (r 32) (g 77) (b 127))"
                         "(a2 (r 62) (g 67) (b 627))";
        auto doc = SDocument::parse(str);
        WHEN("check s expressions") {
            const auto a1 = doc.findChild("a1");
            const auto a2 = doc.findChild("a2");
            THEN("check a1") {
                EXPECT(a1->is<SKind::SEXP>());
                EXPECT(a1->name() == "a1");
            }
            THEN("check a2") {
                EXPECT(a2->is<SKind::SEXP>());
                EXPECT(a2->name() == "a2");
            }
        }
        WHEN("check iterators") {
            const auto a1 = doc.begin();
            const auto a2 = doc.begin() + 1;
                THEN("check a1") {
                    EXPECT(a1->is<SKind::SEXP>());
                    EXPECT(a1->name() == "a1");
                }
                THEN("check a2") {
                    EXPECT(a2->is<SKind::SEXP>());
                    EXPECT(a2->name() == "a2");
                }
        }
        WHEN("query to path") {
            const auto response1 = doc.query("/a1");
            const auto response2 = doc.query("/a2");
            const auto response3 = doc.query("/a3");
            THEN("check path") {
                EXPECT((*response1.begin())->toString() == "(a1 (r 32) (g 77) (b 127))");
                EXPECT((*response2.begin())->toString() == "(a2 (r 62) (g 67) (b 627))");
                EXPECT(response3.empty());
            }
        }
        WHEN("query to path 1 ") {
            const auto response1 = doc.query("/a1/");
            THEN("check path") {
                EXPECT(!response1.empty());
            }
        }
        WHEN("query to path 2 ") {
            const auto response = doc.query("/a3/some");
            THEN("check path") {
                EXPECT(response.empty());
            }
        }
        WHEN("query to path 3 ") {
            const auto response = doc.query("/a1/some");
            THEN("check path") {
                EXPECT(response.empty());
            }
        }
    }
}

SCENARIO("multiply response to query") {
    GIVEN("parse string") {
        const auto str = "(a1 (r 32) (g 77) (b 127))"
                         "(a1 (r 62) (g 67) (b 627))"
                         "(a2 (r 78) (g 14) (b 7))"
                         "(r 82)"
                         "(b (a1 (r 92) (g 17) (b 67)))";

        auto doc = SDocument::parse(str);
        WHEN("query to path 1") {
            auto response = doc.query("/a1");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(a1 (r 32) (g 77) (b 127))");
                it++;
                EXPECT((*it)->toString() == "(a1 (r 62) (g 67) (b 627))");
                it++;
                EXPECT(it == response.end());
            }
        }
        WHEN("query to path 2") {
            auto response = doc.query("/a1/r ");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(r 32)");
                it++;
                EXPECT((*it)->toString() == "(r 62)");
                it++;
                EXPECT(it == response.end());
            }
        }
        WHEN("query to relative ") {
            auto response = doc.query("r");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(r 82)");
                it++;
                EXPECT((*it)->toString() == "(r 32)");
                it++;
                EXPECT((*it)->toString() == "(r 62)");
                it++;
                EXPECT((*it)->toString() == "(r 78)");
                it++;
                EXPECT((*it)->toString() == "(r 92)");
                it++;
                EXPECT(it == response.end());
            }
        }
        WHEN("query to incorrect relative path") {
            auto response = doc.query("r/some/path");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(response.empty());
            }
        }
        WHEN("query to incorrect relative path 2") {
            auto response = doc.query("some/");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(response.empty());
            }
        }
        WHEN("query to path 2") {
            auto response = doc.query("a1/r");
            auto it = response.begin();
            THEN("check path") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(r 32)");
                it++;
                EXPECT((*it)->toString() == "(r 62)");
                it++;
                EXPECT((*it)->toString() == "(r 92)");
                it++;
                EXPECT(it == response.end());
            }
        }
        WHEN("query by filter") {
            auto response = doc.query("[$range={0, 1}]");
            auto it = response.begin();
            THEN("check filter") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(a1 (r 32) (g 77) (b 127))");
                it++;
                EXPECT((*it)->toString() == "(a1 (r 62) (g 67) (b 627))");
                it++;
                EXPECT(it == response.end());
            }
        }
        WHEN("query by filter:range") {
            auto response = doc.query("[$range={6, 100}]");
            THEN("check filter") {
                EXPECT(response.empty());
                EXPECT(response.begin() == response.end());
            }
        }
        WHEN("query by filter:idx") {
            auto response = doc.query("[$idx=0]");
            auto it = response.begin();
            THEN("check filter") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(a1 (r 32) (g 77) (b 127))");
            }
        }
        WHEN("query by filter:idx 1") {
            auto response = doc.query("[$idx=1]");
            auto it = response.begin();
            THEN("check filter") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(a1 (r 62) (g 67) (b 627))");
            }
        }
        WHEN("query by filter:idx 2") {
            auto response = doc.query("[$idx=-1]");
            auto it = response.begin();
            THEN("check filter") {
                EXPECT(!response.empty());
                EXPECT((*it)->toString() == "(b (a1 (r 92) (g 17) (b 67)))");
            }
        }
        WHEN("query by filter:idx 3") {
            auto response = doc.query("[$idx=5]");
            THEN("check filter") {
                EXPECT(response.empty());
            }
        }
        WHEN("query by filter:idx 4") {
            auto response = doc.query("[$idx=-6]");
            THEN("check filter") {
                EXPECT(response.empty());
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (int failures = lest::run(testlist, argc, argv))
        return failures;

    std::cout << "All tests passed\n", EXIT_SUCCESS;
    return 0;
}