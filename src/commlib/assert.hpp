#pragma once
struct Tester {
  const char *file_;
  int line_;
  bool ok_ = true;
  std::string opt;

  Tester(const char *file, int line) : file_(file), line_(line) {};
  ~Tester() {
    if (ok_) { return; }
    std::cout << file_ << ":" << line_ << " failed [" << opt << "] " << std::flush;
    exit(1);
  }
  template<class X>
  inline void ok(const X &x) { ok_ = x ? true : false; }
  template<class X, class Y>
  inline void equal_to(const X &x, const Y &y) {
    ok_ = x == y;
    opt += "==";
  }
  template<class X, class Y>
  inline void not_equal_to(const X &x, const Y &y) {
    ok_ = x != y;
    opt += "!=";
  }
  template<class X, class Y>
  inline void greater_equal(const X &x, const Y &y) {
    ok_ = x >= y;
    opt += ">=";
  }
  template<class X, class Y>
  inline void greater(const X &x, const Y &y) {
    ok_ = x > y;
    opt += ">";
  }
  template<class X, class Y>
  inline void less_equal(const X &x, const Y &y) {
    ok_ = x <= y;
    opt += "<=";
  }
  template<class X, class Y>
  inline void less(const X &x, const Y &y) {
    ok_ = x < y;
    opt += "<";
  }

};

#define ASSERT(b)       Tester(__FILE__, __LINE__).ok(b)
#define ASSERT_EQ(a, b) Tester(__FILE__, __LINE__).equal_to(a, b) // ==
#define ASSERT_NE(a, b) Tester(__FILE__, __LINE__).not_equal_to(a, b) // !=
#define ASSERT_GE(a, b) Tester(__FILE__, __LINE__).greater_equal(a, b) // >=
#define ASSERT_GT(a, b) Tester(__FILE__, __LINE__).greater(a, b) // >
#define ASSERT_LE(a, b) Tester(__FILE__, __LINE__).less_equal(a, b) // <=
#define ASSERT_LT(a, b) Tester(__FILE__, __LINE__).less(a, b) // <