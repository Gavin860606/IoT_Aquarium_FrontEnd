<template>
    <div class="home">
        <v-container>
            <v-row>
                <v-col-6>
                    <div class="title-box">
                        <div class="title">
                            Last Update:
                            <p id="intext">{{ this.last_update }}</p>
                        </div>
                    </div>
                </v-col-6>
                <v-col>
                    <div class="automode">
                        <v-sheet width="100%" height="100%" class="pa-6" style="background-color: rgba(250, 235, 215, 0)">
                            <v-switch v-model="automode" hide-details inset :disabled="toggling" :label="`${mode}`"
                                @change="swtichmode()" color="white">
                            </v-switch>
                        </v-sheet>
                        <v-fade-transition hide-on-leave>

                            <v-card v-if="toggling" append-icon="$close" class="toggle_card" elevation="16" height="200"
                                width="250" color="#fff">
                                <v-progress-circular class="progess" :indeterminate="toggling" :value="20" size="48"
                                    color="blue" width="8"></v-progress-circular>
                                <v-card-text class="sending_direct_method"> Sending Direct Method... </v-card-text>

                            </v-card>
                        </v-fade-transition>
                    </div>
                </v-col>
            </v-row>
            <v-row>
                <v-col-6>
                    <div class="title-box">
                        <div class="title">
                            Temperature:
                            <p id="intext">{{ this.temperature }}°C</p>
                        </div>
                    </div>
                </v-col-6>
                <v-col-6 v-if="!automode">
                    <v-form class="manualMode">
                        <v-row></v-row>
                        <v-textarea class="fan_input" label="Input" auto-grow outlined rows="1" row-height="15"
                            v-model="setPwm"></v-textarea>
                    </v-form>
                    <v-btn class="manulaMode_btn" @click="changeFanSpeed">Send</v-btn>
                </v-col-6>
            </v-row>
            <v-row>
                <v-col>
                    <div class="title">
                        FAN pwm:
                        <p id="intext">{{ this.pwm }}</p>
                    </div>
                </v-col>
            </v-row>
            <v-row>
                <v-col>
                    <div class="bbtn">
                        <v-btn :disabled="btnDisable" @click="update">Refresh</v-btn>
                    </div>
                </v-col>
            </v-row>
        </v-container>
    </div>
</template>

<script>
// @ is an alias to /src

export default {
    name: "Home",
    data() {
        return {
            model: true,
            temperature: 0,
            last_update: 0,
            pwm: 0,
            automode: null,
            btnDisable: false,
            toggling: false,
            setPwm: 0,
        };
    },
    computed: {
        mode() {
            return this.automode ? "Auto Mode" : "Manual Mode";
        },
    },
    watch: {},
    components: {},
    methods: {
        update() {
            this.$axios
                .get(`https://wemosd1dp.azurewebsites.net/`)
                .then((result) => {
                    console.log(result);
                    this.temperature = result.data.temperature.toFixed(2);
                    this.last_update = result.data.last_update;
                    this.pwm = result.data.pwm;
                    this.automode = result.data.automode;
                })
                .catch((err) => {
                    console.log(err);
                });
            // this.btnDisable = !this.btnDisable
        },

        swtichmode() {
            this.toggling = true;
            let mode = this.automode ? 1 : 0;
            console.log("mode:", mode);
            this.$axios
                .post(`http://172.25.205.154:80/api/changemode`, {
                    mode: mode,
                })
                .then((result) => {
                    console.log('changemode API result:', result);
                    if (result.data.payload.status == 'OK') {
                        this.automode = !this.automode;
                        setTimeout(() => {
                            this.toggling = false;
                            this.update()
                        }, 1500);
                    }
                    else {
                        setTimeout(() => {
                            this.toggling = false;
                            this.update()
                        }, 2000);
                        alert('Failed to send direct method')

                    }
                })
                .catch((err) => {
                    setTimeout(() => {
                        this.toggling = false;
                        alert('Failed to send direct method')
                    }, 2000);
                    console.log(err);
                });
        },
        changeFanSpeed() {
            this.toggling = true;
            this.$axios
                .post(`http://172.25.205.154:80/api/changefanspeed`, {
                    fanspeed: parseInt(this.setPwm),
                })
                .then((result) => {
                    console.log(result);
                    this.update()
                    setTimeout(() => {
                        this.toggling = false;
                    }, 1500);
                })
                .catch((err) => {
                    setTimeout(() => {
                        this.toggling = false;
                        alert('Failed to send direct method')
                    }, 2000);
                    console.log(err);
                });
        },

    },
    created() {
        console.log("=== Home created! === ");
        this.$axios
            .get(`https://wemosd1dp.azurewebsites.net/`)
            .then((result) => {
                this.temperature = result.data.temperature.toFixed(2);
                this.last_update = result.data.last_update;
                this.pwm = result.data.pwm;
                this.automode = result.data.automode;
            })
            .catch((err) => {
                console.log(err);
            });
    },
    beforeUpdate() {
        console.log("=== Home beforeUpdate! === ");
    },
    updated() {
        console.log("=== Home updated! === ");
    },
};
</script>
<style lang="scss" scoped>
* {
    // border: 1px solid #f00;
    color: #fff;
}

.home {
    width: 100vw;
    height: 100vh;
    padding-top: 20px;
    // position: relative;
    // background-color: white;
    background-image: linear-gradient(0deg,
            rgb(220, 224, 228) 0%,
            rgb(52, 148, 192) 100%);
    background-size: cover;
}

.title {
    color: rgb(255, 255, 255);
}

#intext {
    font-size: 20px;
}

.bbtn {
    border-radius: 5px;
    height: 25px;
    // margin-left: 40px;
}

.toggle_card {
    position: absolute;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    z-index: 99;
    // height: auto;
    // margin-top: 50px;
}

.automode {
    
    // position: absolute;
    .v-input--selection-controls {
        margin-top: 0;
    }

    // margin-left: 100px;
}

.title-box {
    padding-left: 15px;
}

.btn {
    position: relative;
    // right: 50%;
    top: 20%;
    left: 32%;
}

.manulaMode_btn {
    position: relative;
    top: 12%;
    bottom: 5%;
    left: 30%;
    max-height: 30px;
}

.manualMode {
    position: relative;
    // margin-top: 20px;
    // margin-left: 20px;
    top: 30%;
    left: 30%;
    width: 55%;
}

.fan_input {
    font-size: 18px;
}

.progess {
    left: 40%;
    top: 55%;
}

.sending_direct_method {
    font-size: 19px;
    font-weight: 800;
    position: relative;
}
</style>
