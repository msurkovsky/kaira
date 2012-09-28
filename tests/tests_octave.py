from testutils import Project, RunProgram
import unittest
import os

class OctaveProject(Project):

    def run_test_m(self, expected_output):
        test_m = os.path.join(self.get_directory(), "test.m")
        run = RunProgram("octave", [ "-q", test_m ])
        run.cwd = self.get_directory()
        run.run(expected_output)

    def quick_test_octave(self, expected_output):
        self.build()
        self.run_test_m(expected_output)

class BuildTest(unittest.TestCase):

    def test_octave(self):
        OctaveProject("octave").quick_test_octave(
            "ans = Library is ready\nans =  7\nans =  1\nans =  5\n")

if __name__ == '__main__':
    unittest.main()
