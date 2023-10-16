const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const WasmPackPlugin = require("@wasm-tool/wasm-pack-plugin");
const Dotenv = require('dotenv-webpack');

module.exports = {
  mode: 'development',
  entry: './src/index.ts',
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'bundle.js',
  },
  plugins: [
    new Dotenv(),
    new HtmlWebpackPlugin({
      template: 'src/index.html',
      inject: "body"
    }),
    new WasmPackPlugin({
      crateDirectory: path.resolve(__dirname, '../rs/receiver'),
      outDir: path.resolve(__dirname, './pkg'),
      extraArgs: "--target bundler"
    }),
  ],
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: 'ts-loader',
        exclude: /node_modules/
      },
      {
        test: /\.wasm$/,
        type: 'webassembly/async',
      },
    ],
  },
  resolve: {
    extensions: ['.tsx', '.ts', '.js']
  },
  experiments: {
    asyncWebAssembly: true,
  },
  devServer: {
    allowedHosts: "all",
    static: path.join(__dirname, 'dist'),
    host: "0.0.0.0",
  },
};
