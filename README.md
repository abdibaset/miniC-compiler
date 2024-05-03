<section>
    <h1>Part1: Front End</h1>
    <div>
        <p> The program exits for the following conditions:
        <ul>
            <li>Syntax error by grammar rules. Plan on working the specific errors <em>later</em>.
            <li>Declaration more than once in the same scope.
            <li>Variable referenced before declaration in a valid scope.
        </ul>
        <p> To run the program, <em>after compiling</em>, use the cmnd:
            <code style="font-family: Courier; background-color: #f4f4f4; padding: 0.2em 0.4em; border-radius: 0.3em;"> parser.out parser_tests/p#.c</code> <br>
            To run valgrind while parser dir use the cmnd  <code style="font-family: Courier; background-color: #f4f4f4; padding: 0.2em 0.4em; border-radius: 0.3em;"> ../valgrind.test parser.out parser_tests/p#.c </code>
        </p>
    </div>
</section>

<section>
    <h1>Part3: Optimizations of LLVM modules </h1>
    <p> The programs are organized as follows:
    <div>
        <ul>
            <li>optimizations utils file that contain utility function not necessary related to other modules
            <li>Common Subexpression and Dead code are implemenated in the same module under the utils folder
            <li>Constant folding module in utils folder
            <li>Constant propagation in its own module in utils folder
        </ul>
        <p> <code style="font-family: Courier; background-color: #f4f4f4; padding: 0.2em 0.4em; border-radius: 0.3em;"> optimizations.c </code> file utilizes the modules listed above to analyze a file<br>
        <p> The modules listed are compiled and archived in the lib as <code style="font-family: Courier; background-color: #f4f4f4; padding: 0.2em 0.4em; border-radius: 0.3em;"> libopt.a </code> <br>
    </div>
</section>
