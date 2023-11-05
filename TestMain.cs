using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace testProject {
    internal class TestMain {
        static void Main(string[] args) {
            var testSub = new TestSub();

            testSub.changeInt();

            Console.WriteLine(TestSub.testInt);

            TestSub.testInt = 1;
            Console.WriteLine(TestSub.testInt);
        }
    }
}
