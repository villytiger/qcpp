// Copyright (C) 2015 Ilya Lyubimov
//
// This file is part of qpromise.
//
//  Qpromise is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Qpromise is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with qpromise.  If not, see <http://www.gnu.org/licenses/>.

#define SPECIFY(desc) RecordProperty("description", desc)

#define DESCRIBE(testcase, desc)                                                                                       \
	class testcase : public Test {                                                                                 \
	protected:                                                                                                     \
		static auto describe() { return std::vector<std::string>{desc}; }                                      \
                                                                                                                       \
	public:                                                                                                        \
		static void SetUpTestCase() { RecordProperty("description0", desc); }                                  \
	};

#define DESCRIBE_INNER(testcase, parent, desc)                                                                         \
	class testcase : public parent {                                                                               \
	protected:                                                                                                     \
		static auto describe() {                                                                               \
			auto d = parent::describe();                                                                   \
			d.push_back(desc);                                                                             \
			return d;                                                                                      \
		}                                                                                                      \
                                                                                                                       \
	public:                                                                                                        \
		static void SetUpTestCase() {                                                                          \
			auto description = describe();                                                                 \
			for (size_t i = 0; i != description.size(); ++i) {                                             \
				RecordProperty("description" + std::to_string(i), description[i]);                     \
			}                                                                                              \
		}                                                                                                      \
	};

#define TEST_FULFILLED(testCase, testValue, test)                                                                      \
                                                                                                                       \
	TEST_F(testCase, AlreadyFulfilled) { this->test(Adapter::resolved(testValue)); }                               \
                                                                                                                       \
	TEST_F(testCase, ImmediatelyFulfilled) {                                                                       \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		d.resolve(testValue);                                                                                  \
	}                                                                                                              \
                                                                                                                       \
	TEST_F(testCase, EventuallyFulfilled) {                                                                        \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		Adapter::setTimeout([d]() mutable { d.resolve(testValue); }, 50);                                      \
	}

#define TEST_REJECTED(testCase, testReason, test)                                                                      \
                                                                                                                       \
	TEST_F(testCase, AlreadyRejected) { this->test(Adapter::rejected(testReason)); }                               \
                                                                                                                       \
	TEST_F(testCase, ImmediatelyFulfilled) {                                                                       \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		d.reject(testReason);                                                                                  \
	}                                                                                                              \
                                                                                                                       \
	TEST_F(testCase, EventuallyRejected) {                                                                         \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		Adapter::setTimeout([d]() mutable { d.reject(testReason); }, 50);                                      \
	}
