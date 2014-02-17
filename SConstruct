import os

def which(file):
    for path in os.environ["PATH"].split(":"):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    return None

def abs_path(paths):
    return [Dir(os.path.join(os.getcwd(), path)) for path in paths]

env = DefaultEnvironment()
if ARGUMENTS.get('verbose') != "1":
    env['CCCOMSTR'] = "Compiling $TARGET"
    env['CXXCOMSTR'] = "Compiling $TARGET"
    env['LINKCOMSTR'] = "Linking $TARGET"
    env['ARCOMSTR'] = "Archiving $TARGET"
    env['RANLIBCOMSTR'] = "Making library $TARGET"
env['ENV']['TERM'] = os.environ['TERM']
          
if ARGUMENTS.get('analyze') == "1":
    print("Enabling scan-build analyzer")
    if ARGUMENTS.get('verbose') != "1":
        env['CCCOMSTR'] = "Analyzing $TARGET"
        env['CXXCOMSTR'] = "Analyzing $TARGET"
    env["CC"] = os.getenv("CC") or env["CC"]
    env["CXX"] = os.getenv("CXX") or env["CXX"]
    env["ENV"].update(x for x in os.environ.items() if x[0].startswith("CCC_"))
else:
    if ARGUMENTS.get('gcc') != "1":
        env.Replace(CXX=which('clang++'))
    else:
        env.Replace(CXX=None)
    
if ARGUMENTS.get('asan') != "1":
    env.Append(CXXFLAGS='-g -O3 -finline-functions ')
else:
    env.Append(CXXFLAGS="-g -fsanitize=address")
    env.Append(LINKFLAGS="-fsanitize=address")

env.Append(CPPPATH=["#deps/include", "#src"])
env.Append(LIBPATH="#deps/lib")
env.Append(CXXFLAGS='-std=c++11 -Wall -Wno-deprecated -Wno-write-strings ')

if not env.GetOption('clean'):
    # Perform configuration checks
    conf = Configure(env)
    
    # Check for clang first, if not back off to g++
    if not conf.CheckCXX():
        print("Unable to find clang, checking for g++ instead.")
        env.Replace(CXX=which('g++'))
        env.Append(CXXFLAGS="-mcx16 ")
        if not conf.CheckCXX():
            print('Unable to find a configured C++ compiler.')
            Exit(1)
    else:
        env.Append(CXXFLAGS='-Qunused-arguments ')
            
    # Check for lmdb
    if not conf.CheckCXXHeader('lmdb.h'):
       print 'Did not find lmdb header.'
       Exit(1)
    if not conf.CheckLib('lmdb'):
       print 'Did not find lmdb library.'
       Exit(1)
    # Check for 0mq
    if not conf.CheckCXXHeader('zmq.h'):
       print 'Did not find zmq header.'
       Exit(1)
       
    env = conf.Finish()

env.Clean('distclean', ['.sconsign.dblite', '.sconf_temp', 'config.log'])
libraries = [env.SConscript(['src/transaction/SConscript', 'src/actors/SConscript', 'src/engine/SConscript', 'src/mbox/SConscript', 'src/decimal/SConscript'], exports='env')]
#env.SConscript(['src/SConscript', 'tests/SConscript'], exports=['env', 'libraries'])
env.SConscript(['src/SConscript'], exports=['env', 'libraries'])
