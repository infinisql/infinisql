import os

def which(file):
    for path in os.environ["PATH"].split(":"):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    return None

env = DefaultEnvironment()
if ARGUMENTS.get('VERBOSE') != "1":
  env['CCCOMSTR'] = "Compiling $TARGET"
  env['LINKCOMSTR'] = "Linking $TARGET"
env['ENV']['TERM'] = os.environ['TERM']
          
env.Replace(CXX=which('clang++'))
env.Append(LIBPATH="../deps/lib")
env.Append(CCFLAGS="-I../deps/include")

if not env.GetOption('clean'):
    # Perform configuration checks
    conf = Configure(env)
    
    # Check for clang first, if not back off to g++
    if not conf.CheckCXX():
        print("Unable to find clang, checking for g++ instead.")
        env.Replace(CXX=which('g++'))
        if not conf.CheckCXX():
            print('Unable to find a configured C++ compiler.')
            Exit(1)
            
    # Check for lmdb
    if not conf.CheckLibWithHeader('lmdb', 'lmdb.h', 'c++'):
       print 'Did not find either lmdb library, header or both, exiting!'
       Exit(1)
       
    env = conf.Finish()

env.Clean('distclean', ['.sconsign.dblite', '.sconf_temp', 'config.log'])
    
SConscript(['src/SConscript', 'tests/SConscript'], exports='env')

