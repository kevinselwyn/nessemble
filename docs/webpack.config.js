const path = require('path');

module.exports = {
    entry: './js/index.ts',
    output: {
        path: path.resolve('pages/js'),
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
    devtool: 'eval-cheap-module-source-map'
};
