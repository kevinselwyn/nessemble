const path = require('path');
const fs = require('fs');

const OnlyIfChangedWebpackPlugin = require('only-if-changed-webpack-plugin');

const rootDir = path.resolve('pages/js');
const cacheDir = path.resolve('cache');

try {
    fs.mkdirSync(cacheDir);
} catch (e) {}

module.exports = {
    entry: './js/index.ts',
    output: {
        path: rootDir,
        filename: 'bundle.js'
    },
    resolve: {
        extensions: ['.ts', '.tsx', '.js', '.jsx']
    },
    module: {
        loaders: [
            {
                test: /\.j(s|sx)$/,
                loader: 'babel-loader',
                exclude: /node_modules/
            },
            {
                test: /\.t(s|sx)$/,
                loader: 'ts-loader',
                exclude: /node_modules/
            }
        ]
    },
    plugins: [
        new OnlyIfChangedWebpackPlugin({
            cacheDirectory: cacheDir,
            cacheIdentifier: {
                rootDir: rootDir,
                devBuild: process.env.NODE_ENV !== 'production'
            }
        })
    ],
    devtool: 'eval-cheap-module-source-map'
};
