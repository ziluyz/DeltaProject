int execute(int argc, char** argv, int (*f)(), void* data) { // begin main execution cycle
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);
    auto &d = *static_cast<Data*>(data);