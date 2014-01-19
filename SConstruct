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

if ARGUMENTS.get('asan') != "1":
    env.Append(CXXFLAGS='-g -O3 -finline-functions ')
else:
    env.Append(CXXFLAGS="-g -fsanitize=address")
    env.Append(LINKFLAGS="-fsanitize=address")
          
env.Replace(CXX=which('clang++'))
env.Append(CPPPATH="#deps/include")
env.Append(LIBPATH="#deps/lib")
env.Append(CXXFLAGS='-std=c++11 -Wall -Wno-deprecated -Wno-write-strings -Qunused-arguments ')

if not env.GetOption('clean'):
    # Perform configuration checks
    conf = Configure(env)
    
    # Check for clang first, if not back off to g++
    if not conf.CheckCXX():
        print("Unable to find clang, checking for g++ instead.")
        env.Replace(CXX=which('g++'))
        env.Append(CXXFLAGS='-mcx16')
        if not conf.CheckCXX():
            print('Unable to find a configured C++ compiler.')
            Exit(1)
            
    # Check for lmdb
    if not conf.CheckCXXHeader('lmdb.h'):
       print 'Did not find lmdb header.'
       Exit(1)
    if not conf.CheckLib('lmdb'):
       print 'Did not find lmdb library.'
       Exit(1)
       
    env = conf.Finish()

env.Clean('distclean', ['.sconsign.dblite', '.sconf_temp', 'config.log'])
env.SConscript(['src/SConscript', 'tests/SConscript'], exports='env')
